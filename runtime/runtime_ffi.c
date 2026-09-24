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
// ============================================================
// M200（缺陷 240）：把 FFI 表里的名字**发布成全局 native**
// ------------------------------------------------------------
// 为什么需要（这是官方 registry 的 `zlib` 包在**编译轨**完全不可用的根因）：
//   `extern def` 声明的名字（`import "c/zlib"` + `extern def zlib_compress(...)`）在
//   **解释轨**有运行期兜底 —— `selfhost/iexpr.px` 遇到未知名/FFI 名会调
//   `i_builtin_ffi_call` → C 侧 `ffi_call` 的**双表**（ffi 注册表 → 全局 native 表）；
//   而**编译轨**把这个名字编译成 **GETG**（extern def 名即全局名）⇒ GETG 只查全局表 ⇒
//   `R1001 未定义变量: 'zlib_compress'`。
//   实证（M200）：
//     · 解释轨：`print(zlib_crc32(bytes("abc")))` → `891568578` ✅
//     · 编译轨：同一条 → `运行时错误 [zl_compress 行22]: R1001: 未定义变量: 'zlib_compress'`
//       （默认档与 `--full` 档**皆然** ⇒ 与「按引用集自动裁剪」无关，是**发布缺失**）
//   后果：官方 `registry/zlib` 在 `px build` 产物里不可用（第三方把它记成 PX-DEF-035，
//   且方向记反了：他们以为"编译轨可用、解释轨未注册"）。
//   ⇒ 三轨同一条真相：注册完成后把每个 FFI 名发布成全局 native。
// ⚠️ 调用时机：必须在**全部** `px_ffi_register` 之后（当前唯一调用点 = `px_register_builtins` 末尾，
//   且仍在 `g_gc_frozen = 1` 的建表窗口内 ⇒ 不会被 GC 误回收，见 M170 缺陷 189）。
// ============================================================
void px_ffi_publish_globals(void) {
    int i;
    for (i = 0; i < g_ffi_n; i++) {
        px_set_global(g_ffi_syms[i].name, px_native(g_ffi_syms[i].name, g_ffi_syms[i].fn));
    }
}

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
// ==================== M158（第 40 轮 · 缺陷 114 根治）：解释轨函数值桥 ====================
// 背景：解释轨（bootstrap/pxi）把用户函数包装成 dict（{"__ufn__": {...}} /
//   {"__builtin__": name}），而 runtime 的 native（set_interval / set_timeout /
//   signal / http_serve / http_serve_unix / http_stream / sse_serve / ws_serve /
//   udp_serve / http_get_stream / route / middleware …）一律按 PX_FUNC/PX_NATIVE
//   校验函数参数 ⇒ 解释轨把包装 dict 直接转发 ⇒ px_error「R1002: set_interval:
//   第一个参数必须是函数」（**不可捕获**，直接终止进程）——即缺陷 114：解释轨
//   用户代码里「把函数值传给 runtime native」整族不可用。
// 方案（双向桥，零语言语法改动）：
//   ① 桥对象 = px_func_env(name, px_interp_bridge_entry, fnvalue)：复用 M129 的 env
//      字段承载解释轨函数值（PX_FUNC 对象被 GC 标记 ⇒ env 自动可达 ⇒ 无需额外
//      注册表、不泄漏；ctx 由 px_func_env 指向 env 字段 ⇒ entry 取回原值）。
//   ② 调度器 = 解释器启动时 interp_bridge_install(cb) 把**它自己的**编译版函数
//      i_bridge_dispatch 存进全局表 __interp_dispatch__（全局表 = GC 根）。
//   ③ 桥 entry 被 native 调用 ⇒ 取回 fnvalue ⇒ px_call(dispatcher, [fnvalue,
//      args_list]) ⇒ 回到解释器执行（解释轨的真闭包 / env 链语义完整保留）。
//   ④ bi_ffi_call **自动桥接**：解释轨裸脚本（零 extern def）调 runtime native 走
//      ffi_call 按名兜底，此处对参数列表做「包装 dict → 桥」替换 ⇒ 一处修复、全族生效。
// 边界：调度器全局唯一（同进程多解释器实例不支持）；桥被 native 跨线程调用时解释器
//   状态非线程安全（与编译模式"用户自负责"同级，登记为边界）。

// 是否解释轨函数值包装（用户函数 / 内置函数）
static int px_interp_is_fnwrap(LXValue v) {
    if (v.type != PX_DICT) return 0;
    return px_dict_has(v, "__ufn__") || px_dict_has(v, "__builtin__");
}

// 包装里的名字（桥对象显示名；取不到给 interp-fn）
static const char* px_interp_fnwrap_name(LXValue v) {
    LXValue inner = px_dict_get(v, "__ufn__");
    if (inner.type == PX_DICT) {
        LXValue nm = px_dict_get(inner, "name");
        if (nm.type == PX_STR) return nm.as.obj->as.str.data;
    }
    LXValue bn = px_dict_get(v, "__builtin__");
    if (bn.type == PX_STR) return bn.as.obj->as.str.data;
    return "interp-fn";
}

// 桥 entry：被 runtime（native / 定时器 / 服务循环）回调 ⇒ 转回解释器执行
static LXValue px_interp_bridge_entry(LXValue* args, int nargs, void* ctx) {
    if (!ctx) px_error("R1002: 解释轨函数桥缺少上下文");
    LXValue fnvalue = *(LXValue*)ctx;                     // px_func_env 把 ctx 指向 env 字段
    LXValue disp = px_get_global("__interp_dispatch__");   // 未安装 ⇒ 未定义变量（明确）
    if (disp.type != PX_FUNC && disp.type != PX_NATIVE) {
        px_error("R1002: 解释轨函数桥的调度器不是函数（interp_bridge_install）");
    }
    LXValue lst = px_list(nargs);
    LXValue callargs[2];
    callargs[0] = fnvalue;
    callargs[1] = lst;
    px_root_push();
    PX_KEEP(callargs[0]);
    PX_KEEP(callargs[1]);
    for (int i = 0; i < nargs; i++) {
        PX_KEEP(args[i]);
        px_list_push(lst, args[i]);
    }
    LXValue r = px_call(disp, callargs, 2);
    px_root_pop();
    return r;
}

// 显式造桥（解释轨可主动调用；已是真函数值则原样返回）
LXValue bi_interp_bridge(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("R1002: interp_bridge 需要 1 个参数（解释轨函数值）");
    if (!px_interp_is_fnwrap(args[0])) return args[0];
    return px_func_env(px_interp_fnwrap_name(args[0]), px_interp_bridge_entry, args[0]);
}

// 安装调度器（解释器启动时调用一次）
LXValue bi_interp_bridge_install(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("R1002: interp_bridge_install 需要 1 个参数（调度函数）");
    if (args[0].type != PX_FUNC && args[0].type != PX_NATIVE) {
        px_error("R1002: interp_bridge_install 的参数必须是函数");
    }
    px_set_global("__interp_dispatch__", args[0]);
    return px_bool(true);
}

LXValue bi_ffi_call(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("R1002: ffi_call 需要 (name, args_list) 参数");
    if (args[0].type != PX_STR) px_error("R1002: ffi_call 的 name 需要字符串");
    if (args[1].type != PX_LIST) px_error("R1002: ffi_call 的 args 需要列表");
    const char* name = args[0].as.obj->as.str.data;
    LXObject*   lst  = args[1].as.obj;
    int         i;
    // M158：先解析目标绑定（两表兜底顺序与语义不变：① ffi 表 ② 宿主全局 native 表）
    LXFuncPtr target = NULL;
    for (i = 0; i < g_ffi_n; i++) {
        if (strcmp(g_ffi_syms[i].name, name) == 0) { target = g_ffi_syms[i].fn; break; }
    }
    if (!target) {
        LXValue gv;
        if (px_global_native(name, &gv)) target = gv.as.obj->as.native.fn;
    }
    if (!target) {
        char msg[512];
        snprintf(msg, sizeof(msg), "ffi_call: 未注册函数: %s", name);
        return px_err(px_str(msg));
    }
    // M158（第 40 轮 · 缺陷 114）：解释轨函数值桥 —— 一处修复、全族 native 生效。
    //   无包装参数时零额外开销（一次线性探测后直调原路径，语义与改前逐字节相同）。
    LXValue* items = lst->as.list.items;
    int      n     = lst->as.list.len;
    int      need  = 0;
    for (i = 0; i < n; i++) { if (px_interp_is_fnwrap(items[i])) { need = 1; break; } }
    if (!need) return target(items, n, NULL);
    px_root_push();
    LXValue nl = px_list(n);
    PX_KEEP(nl);
    for (i = 0; i < n; i++) {
        LXValue v = items[i];
        if (px_interp_is_fnwrap(v)) {
            v = px_func_env(px_interp_fnwrap_name(v), px_interp_bridge_entry, v);
        }
        PX_KEEP(v);
        px_list_push(nl, v);
    }
    LXValue r = target(nl.as.obj->as.list.items, n, NULL);
    px_root_pop();
    return r;
}

// FFI 符号是否已注册（语言层 extern 调用前的运行时诊断）
bool px_ffi_has(const char* name) {
    int i;
    for (i = 0; i < g_ffi_n; i++) {
        if (strcmp(g_ffi_syms[i].name, name) == 0) return true;
    }
    return false;
}
