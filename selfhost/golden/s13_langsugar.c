/* 由普贤 (PuXian) 编译器自动生成 — px build */
#include "runtime.h"
#include <string.h>
#include <stdio.h>


static LXValue fn_level_tag(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("level_tag");
    LXValue _v1 = (nargs > 0) ? args[0] : px_null();
    LXValue _v2 = px_null();
    LXValue px_err_3_val = px_null();
    int px_err_3_proped = 0;
    px_srcline(9);
    _v2 = ({ LXValue _t4 = _v1; if (px_is_truthy(px_eq(_t4, px_str("info")))) { _t4 = ({ LXValue _blk = px_null(); _blk = px_str("I"); _blk; }); } else if (px_is_truthy(px_eq(_t4, px_str("warn")))) { _t4 = ({ LXValue _blk = px_null(); _blk = px_str("W"); _blk; }); } else if (px_is_truthy(px_eq(_t4, px_str("error")))) { _t4 = ({ LXValue _blk = px_null(); _blk = px_str("E"); _blk; }); } else if (true) { _t4 = ({ LXValue _blk = px_null(); _blk = px_str("?"); _blk; }); } _t4; });
    px_srcline(18);
    return _v2;
px_err_3:
    if (px_err_3_proped) return px_err_3_val;
    return px_null();
}

static LXValue fn_main(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    px_srcfunc("main");
    LXValue _v5 = px_null();
    LXValue _v6 = px_null();
    LXValue _v7 = px_null();
    LXValue _v8 = px_null();
    LXValue _v9 = px_null();
    LXValue _v10 = px_null();
    LXValue _v11 = px_null();
    LXValue _v12 = px_null();
    LXValue px_err_13_val = px_null();
    int px_err_13_proped = 0;
    px_srcline(31);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("info")}, 1));
    px_srcline(32);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("warn")}, 1));
    px_srcline(33);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("error")}, 1));
    px_srcline(34);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("eq: "), px_call(px_get_global("str"), (LXValue[]){px_eq(px_str("info"), px_str("info"))}, 1))}, 1));
    px_srcline(35);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("tag I: "), px_call(px_get_global("level_tag"), (LXValue[]){px_str("info")}, 1))}, 1));
    px_srcline(36);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("tag W: "), px_call(px_get_global("level_tag"), (LXValue[]){px_str("warn")}, 1))}, 1));
    px_srcline(37);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("tag E: "), px_call(px_get_global("level_tag"), (LXValue[]){px_str("error")}, 1))}, 1));
    px_srcline(38);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("tag ?: "), px_call(px_get_global("level_tag"), (LXValue[]){px_str("other")}, 1))}, 1));
    px_srcline(41);
    px_srcline(42);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("code A: "), px_call(px_get_global("str"), (LXValue[]){px_int(1LL)}, 1))}, 1));
    px_srcline(43);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("code B: "), px_call(px_get_global("str"), (LXValue[]){px_int(2LL)}, 1))}, 1));
    px_srcline(44);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("code OK: "), px_call(px_get_global("str"), (LXValue[]){px_bool(true)}, 1))}, 1));
    px_srcline(47);
    _v5 = px_enum("Kind", "Str");
    px_srcline(48);
    _v6 = ({ LXValue _t14 = _v5; if ((_t14.type == PX_ENUM && strcmp(_t14.as.obj->as.enum_inst.variant, "Int") == 0)) { _t14 = ({ LXValue _blk = px_null(); _blk = px_str("int"); _blk; }); } else if ((_t14.type == PX_ENUM && strcmp(_t14.as.obj->as.enum_inst.variant, "Str") == 0)) { _t14 = ({ LXValue _blk = px_null(); _blk = px_str("str"); _blk; }); } _t14; });
    px_srcline(53);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("kind: "), _v6)}, 1));
    px_srcline(56);
    _v7 = px_list_n((LXValue[]){}, 0);
    px_srcline(57);
    (void)(px_method(_v7, "append", (LXValue[]){px_int(1LL)}, 1));
    px_srcline(58);
    (void)(px_method(_v7, "append", (LXValue[]){px_int(2LL)}, 1));
    px_srcline(59);
    (void)(px_method(_v7, "append", (LXValue[]){px_int(3LL)}, 1));
    px_srcline(60);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("a: "), px_call(px_get_global("str"), (LXValue[]){_v7}, 1))}, 1));
    px_srcline(63);
    _v8 = px_list_n((LXValue[]){}, 0);
    px_srcline(64);
    (void)(px_method(_v8, "append", (LXValue[]){px_int(1LL)}, 1));
    px_srcline(65);
    (void)(px_method(_v8, "append", (LXValue[]){px_int(2LL)}, 1));
    px_srcline(66);
    (void)(px_method(_v8, "append", (LXValue[]){px_int(3LL)}, 1));
    px_srcline(67);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("b: "), px_call(px_get_global("str"), (LXValue[]){_v8}, 1))}, 1));
    px_srcline(68);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("append eq: "), px_call(px_get_global("str"), (LXValue[]){px_eq(_v7, _v8)}, 1))}, 1));
    px_srcline(71);
    _v9 = px_list_n((LXValue[]){px_list_n((LXValue[]){}, 0), px_list_n((LXValue[]){}, 0)}, 2);
    px_srcline(72);
    (void)(px_method(px_index(_v9, px_int(0LL)), "append", (LXValue[]){px_int(10LL)}, 1));
    px_srcline(73);
    (void)(px_method(px_index(_v9, px_int(1LL)), "append", (LXValue[]){px_int(20LL)}, 1));
    px_srcline(74);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("rows: "), px_call(px_get_global("str"), (LXValue[]){_v9}, 1))}, 1));
    px_srcline(75);
    _v10 = ({ LXValue _d = px_dict(); { LXValue _k = px_str("k"); if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, px_list_n((LXValue[]){}, 0)); } _d; });
    px_srcline(76);
    (void)(px_method(px_index(_v10, px_str("k")), "append", (LXValue[]){px_int(99LL)}, 1));
    px_srcline(77);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("m: "), px_call(px_get_global("str"), (LXValue[]){_v10}, 1))}, 1));
    px_srcline(80);
    _v11 = px_struct("Bag", (char*[]){"items"}, (LXValue[]){px_list_n((LXValue[]){}, 0)}, 1);
    px_srcline(81);
    (void)(px_method(px_field(_v11, "items"), "append", (LXValue[]){px_int(7LL)}, 1));
    px_srcline(82);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("bag: "), px_call(px_get_global("str"), (LXValue[]){px_field(_v11, "items")}, 1))}, 1));
    px_srcline(85);
    _v12 = px_int(5LL);
    px_srcline(86);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp: "), px_call(px_get_global("str"), (LXValue[]){px_lt(_v12, px_neg(px_int(3LL)))}, 1))}, 1));
    px_srcline(87);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_add(px_str("cmp2: "), px_call(px_get_global("str"), (LXValue[]){px_lt(_v12, px_neg(px_int(1LL)))}, 1))}, 1));
    px_srcline(88);
    (void)(px_call(px_get_global("print"), (LXValue[]){px_str("sugar ok")}, 1));
px_err_13:
    if (px_err_13_proped) return px_err_13_val;
    return px_null();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_set_global("level_tag", px_func("level_tag", fn_level_tag, NULL));
    px_set_global("main", px_func("main", fn_main, NULL));
    px_srcline(6);
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
