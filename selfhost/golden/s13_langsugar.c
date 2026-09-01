#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_level_tag(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue px_err_2_val = px_null();
    int px_err_2_proped = 0;
    LXValue _v4 = ({ LXValue _t3 = _v1; if (px_is_truthy(px_eq(_t3, px_str("info")))) { _t3 = ({ LXValue _blk = px_null(); _blk = px_str("I"); _blk; }); } else if (px_is_truthy(px_eq(_t3, px_str("warn")))) { _t3 = ({ LXValue _blk = px_null(); _blk = px_str("W"); _blk; }); } else if (px_is_truthy(px_eq(_t3, px_str("error")))) { _t3 = ({ LXValue _blk = px_null(); _blk = px_str("E"); _blk; }); } else if (true) { _t3 = ({ LXValue _blk = px_null(); _blk = px_str("?"); _blk; }); } _t3; });
    return _v4;
px_err_2:
    if (px_err_2_proped) return px_err_2_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    LXValue _v5 = px_null();
    LXValue px_err_6_val = px_null();
    int px_err_6_proped = 0;
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("info")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("warn")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("error")}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("eq: "), px_call(px_get_global("str"), (LXValue[]){px_eq(px_str("info"), px_str("info"))}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("tag I: "), px_call(px_get_global("level_tag"), (LXValue[]){px_str("info")}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("tag W: "), px_call(px_get_global("level_tag"), (LXValue[]){px_str("warn")}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("tag E: "), px_call(px_get_global("level_tag"), (LXValue[]){px_str("error")}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("tag ?: "), px_call(px_get_global("level_tag"), (LXValue[]){px_str("other")}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("code A: "), px_call(px_get_global("str"), (LXValue[]){px_int(1LL)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("code B: "), px_call(px_get_global("str"), (LXValue[]){px_int(2LL)}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("code OK: "), px_call(px_get_global("str"), (LXValue[]){px_bool(true)}, 1))}, 1));
    LXValue _v7 = px_enum("Kind", "Str");
    LXValue _v9 = ({ LXValue _t8 = _v7; if ((_t8.type == PX_ENUM && strcmp(_t8.as.obj->as.enum_inst.variant, "Int") == 0)) { _t8 = ({ LXValue _blk = px_null(); _blk = px_str("int"); _blk; }); } else if ((_t8.type == PX_ENUM && strcmp(_t8.as.obj->as.enum_inst.variant, "Str") == 0)) { _t8 = ({ LXValue _blk = px_null(); _blk = px_str("str"); _blk; }); } _t8; });
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("kind: "), _v9)}, 1));
    LXValue _v10 = px_list_n((LXValue[]){}, 0);
    (void)(px_method(_v10, "append", (LXValue[]){px_int(1LL)}, 1));
    (void)(px_method(_v10, "append", (LXValue[]){px_int(2LL)}, 1));
    (void)(px_method(_v10, "append", (LXValue[]){px_int(3LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("a: "), px_call(px_get_global("str"), (LXValue[]){_v10}, 1))}, 1));
    LXValue _v11 = px_list_n((LXValue[]){}, 0);
    (void)(px_method(_v11, "append", (LXValue[]){px_int(1LL)}, 1));
    (void)(px_method(_v11, "append", (LXValue[]){px_int(2LL)}, 1));
    (void)(px_method(_v11, "append", (LXValue[]){px_int(3LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("b: "), px_call(px_get_global("str"), (LXValue[]){_v11}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("append eq: "), px_call(px_get_global("str"), (LXValue[]){px_eq(_v10, _v11)}, 1))}, 1));
    LXValue _v12 = px_list_n((LXValue[]){px_list_n((LXValue[]){}, 0), px_list_n((LXValue[]){}, 0)}, 2);
    (void)(px_method(px_index(_v12, px_int(0LL)), "append", (LXValue[]){px_int(10LL)}, 1));
    (void)(px_method(px_index(_v12, px_int(1LL)), "append", (LXValue[]){px_int(20LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("rows: "), px_call(px_get_global("str"), (LXValue[]){_v12}, 1))}, 1));
    LXValue _v13 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("k"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){}, 0)); } _d; });
    (void)(px_method(px_index(_v13, px_str("k")), "append", (LXValue[]){px_int(99LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("m: "), px_call(px_get_global("str"), (LXValue[]){_v13}, 1))}, 1));
    LXValue _v14 = px_struct("Bag", (char*[]){"items"}, (LXValue[]){px_list_n((LXValue[]){}, 0)}, 1);
    (void)(px_method(px_field(_v14, "items"), "append", (LXValue[]){px_int(7LL)}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("bag: "), px_call(px_get_global("str"), (LXValue[]){px_field(_v14, "items")}, 1))}, 1));
    LXValue _v15 = px_int(5LL);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp: "), px_call(px_get_global("str"), (LXValue[]){px_lt(_v15, px_neg(px_int(3LL)))}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp2: "), px_call(px_get_global("str"), (LXValue[]){px_lt(_v15, px_neg(px_int(1LL)))}, 1))}, 1));
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("sugar ok")}, 1));
px_err_6:
    if (px_err_6_proped) return px_err_6_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("level_tag", px_func("level_tag", fn_level_tag, NULL));
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
