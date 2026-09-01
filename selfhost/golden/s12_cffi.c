/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue px_err_1_val = px_null();
    int px_err_1_proped = 0;
    LXValue _v2 = px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_open"), px_list_n((LXValue[]){px_str(":memory:")}, 1)}, 2);
    (void)(px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_exec"), px_list_n((LXValue[]){_v2, px_str("CREATE TABLE t (id INT, name TEXT)")}, 2)}, 2));
    (void)(px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_exec"), px_list_n((LXValue[]){_v2, px_str("INSERT INTO t VALUES (1, '甲'), (2, '乙')")}, 2)}, 2));
    LXValue _v3 = px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_query"), px_list_n((LXValue[]){_v2, px_str("SELECT id, name FROM t ORDER BY id")}, 2)}, 2);
    LXValue _t4 = _v3;
    for (int _t5 = 0; _t5 < px_len(_t4); _t5++) {
        LXValue _v6 = px_index(_t4, px_int(_t5));
        (void)(px_call(px_get_global("print"), (LXValue[]){px_index(_v6, px_str("id")), px_index(_v6, px_str("name"))}, 2));
    }
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("escape: "), px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_escape"), px_list_n((LXValue[]){px_str("O'Reilly")}, 1)}, 2))}, 1));
    LXValue _v7 = px_call(px_get_global("ffi_call"), (LXValue[]){px_str("sqlite_close"), px_list_n((LXValue[]){_v2}, 1)}, 2);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("closed: "), px_call(px_get_global("str"), (LXValue[]){_v7}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("ffi ok")}, 1));
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
