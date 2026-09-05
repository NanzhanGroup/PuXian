// 普贤 (PuXian) C 运行时 — FFI 注册表（M42 显式 C 库 import）
// ------------------------------------------------------------
// 语言层：
//   import "c/xxx"         声明导入 C 库（不加载 .px 文件）
//   extern def name(a, b) : ret    C 函数声明（无 body）
// 调用：extern 函数调用 → C 桥 builtin ffi_call(name, args_list)
//   编译模式：codegen 生成 px_call(px_get_global("ffi_call"), ["name", [args]], 2)
//   解释模式：i_builtin_ffi_call → 同一 C 桥
// 机制：注册表 {name → LXFuncPtr}；绑定函数（bi_*）为 LXValue 接口，
//       参数/返回值天然支持 int/float/str/bytes/list/dict/ptr(null)，
//       与 runtime_*.c 现有绑定完全同构（M42.3 类型映射 MVP）。
// 新增 C 库 = runtime_ffi_xxx.c 绑定 + px_ffi_register + 语言层 extern 声明。
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FFI_SYMS 256

typedef struct {
    const char* name;
    LXFuncPtr   fn;
} FFISym;

static FFISym g_ffi_syms[MAX_FFI_SYMS];
static int    g_ffi_n = 0;

// 注册 C 绑定到 FFI 表（px_register_builtins 或绑定文件初始化时调用）
void px_ffi_register(const char* name, LXFuncPtr fn) {
    if (g_ffi_n >= MAX_FFI_SYMS) {
        fprintf(stderr, "[ffi] 注册表已满（%d），忽略 %s\n", MAX_FFI_SYMS, name);
        return;
    }
    g_ffi_syms[g_ffi_n].name = name;
    g_ffi_syms[g_ffi_n].fn   = fn;
    g_ffi_n++;
}

// C 桥 builtin：ffi_call(name, args_list) → 查表调用绑定函数
// 双模式统一入口（编译模式 codegen 生成 / 解释模式 i_builtin_ffi_call 调）
// M68 双表兜底：① ffi 注册表（extern def C 库：zlib/sqlite/bytes 转换等）→
// ② 宿主全局 PX_NATIVE 表（px_set_global 注册的全部 runtime 内置，编译产物
//    裸名调用即 px_get_global 查此表）→ pxi 零 extern def 裸脚本可达性对齐。
// 两表均未命中 → 返回 px_err（可辨错误，不杀进程）：pxi 回退据此转 R1001，
// 真拼错名不误调、错误语义不漂移。
LXValue bi_ffi_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("ffi_call 需要 (name, args_list) 参数");
    if (args[0].type != PX_STR) px_error("ffi_call 的 name 需要字符串");
    if (args[1].type != PX_LIST) px_error("ffi_call 的 args 需要列表");
    const char* name = args[0].as.obj->as.str.data;
    LXObject*   lst  = args[1].as.obj;
    int         i;
    for (i = 0; i < g_ffi_n; i++) {
        if (strcmp(g_ffi_syms[i].name, name) == 0) {
            return g_ffi_syms[i].fn(lst->as.list.items, lst->as.list.len, NULL);
        }
    }
    LXValue gv;
    if (px_global_native(name, &gv)) {
        LXFuncPtr fn = gv.as.obj->as.native.fn;
        return fn(lst->as.list.items, lst->as.list.len, NULL);
    }
    char msg[512];
    snprintf(msg, sizeof(msg), "ffi_call: 未注册函数: %s", name);
    return px_err(px_str(msg));
}

// FFI 符号是否已注册（语言层 extern 调用前的运行时诊断）
bool px_ffi_has(const char* name) {
    int i;
    for (i = 0; i < g_ffi_n; i++) {
        if (strcmp(g_ffi_syms[i].name, name) == 0) return true;
    }
    return false;
}
