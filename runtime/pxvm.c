// ============================================================
// M442-S2: runtime/pxvm.c —— .pxe 宿主（一份 runtime 加载任意多模块）
// ------------------------------------------------------------
// 语义与静态产物**同源**：main 体与 selfhost/bc_emit.px 生成的 C 产物逐句对应，
//   只差「模块从哪来」（静态编译期数组 ⇄ .pxe 运行期加载）。
//   ⇒ 门必须做三轨等价：解释轨 / 静态 VM-C 轨 / .pxe + pxvm 轨，覆盖 ≥10 样例。
// 用法：
//   pxvm <module.pxe> [程序参数...]     跑模块（退出码语义同静态产物）
//   pxvm --info <module.pxe>            只打印模块摘要（不执行）
// 退出码：程序自身的退出码；加载失败返回 PXE_* （2..8），用法错误 64。
// ============================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pxe.h"
#include "runtime.h"
#include "vm.h"

static int usage(void) {
    fprintf(stderr,
            "用法: pxvm <module.pxe> [程序参数...]\n"
            "      pxvm --info <module.pxe>\n"
            "退出码: 程序退出码 | 加载失败 = 2 版本 / 3 布局 / 4 指令 / 5 字符串 / 6 校验和 / 7 IO / 8 内存\n");
    return 64;
}

int main(int argc, char** argv) {
    if (argc < 2) return usage();

    int ai = 1;
    int info = 0;
    if (strcmp(argv[1], "--info") == 0) {
        info = 1;
        ai = 2;
        if (ai >= argc) return usage();
    }
    const char* path = argv[ai];

    PxeModule pm;
    PxeErr e;
    int rc = pxe_load(path, &pm, &e);
    if (rc != PXE_OK) {
        fprintf(stderr, "pxvm: 加载失败 [%s] %s: %s\n", path, pxe_rc_name(rc), e.msg);
        return rc;
    }
    if (info) {
        pxe_info(&pm);
        pxe_free(&pm);
        return 0;
    }

    // ---- 以下与 bc_emit 生成的静态产物 main 体逐句对应 ----
    px_args_init(argc - ai, argv + ai);   // argv[0] = 模块路径（与静态产物 argv[0]=程序路径 对齐）
    px_register_builtins();
    px_gc_set_precise(1);
    LXValue _r = px_vm_run_module(px_vm_state(), &pm.m);
    int _code = 0;
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
    if (getenv("PX_BC_DUMP")) {
        for (int _i = 0; _i < (int)pm.m.nG; _i++) {
            LXValue _v = px_get_global(pm.m.G[_i]);
            if (_v.type != PX_FUNC && _v.type != PX_NATIVE)
                printf("%s=%s\n", pm.m.G[_i], px_to_string(_v));
        }
    }
    pxe_free(&pm);
    return px_exit_code_final(_code);
}
