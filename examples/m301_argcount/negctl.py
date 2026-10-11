#!/usr/bin/env python3
# ============================================================
# M301 负控打桩器：快照 / 忠实撤回 / 还原 / 还原校验
# ------------------------------------------------------------
# 纪律（M286 A4 + M223/M226 教训）：
#   · 每个改动都**先断言锚点唯一命中**（sed 静默空转会让负控**丢牙**而门仍报绿）；
#   · 还原后**逐字节比对**快照（还原保真）；
#   · 「撤回」必须是**语义上的退回**，不是换个写法。
# 用法：negctl.py {snapshot|negA_on|negB_on|restore|check}
# ============================================================
import os, shutil, sys

ROOT = os.environ.get('M301_ROOT') or os.getcwd()
SNAP = os.environ.get('M301_SNAP') or '/tmp/m301_gate/snap'
VM = os.path.join(ROOT, 'runtime/vm.c')
CG = os.path.join(ROOT, 'selfhost/codegen.px')

FILES = [VM, CG]


def rd(p):
    return open(p, encoding='utf-8').read()


def wr(p, s):
    open(p, 'w', encoding='utf-8').write(s)


def one(s, old, new, tag):
    n = s.count(old)
    if n != 1:
        print('❌ [%s] 锚点命中 %d 次（应为 1）' % (tag, n))
        sys.exit(1)
    return s.replace(old, new)


CMD = sys.argv[1] if len(sys.argv) > 1 else 'check'

if CMD == 'snapshot':
    os.makedirs(SNAP, exist_ok=True)
    for f in FILES:
        shutil.copy2(f, os.path.join(SNAP, os.path.basename(f)))
    print('M301-NEG-SNAPSHOT-OK %d 件' % len(FILES))

elif CMD == 'negA_on':
    # 负控 A：把 VM 轨的实参个数判据**整体停用**（忠实退回「不查」），
    #   保留函数与调用点 ⇒ 编译通过、行为退回修前。
    s = rd(VM)
    s = one(s, 'static void vm_check_arity(const PxVMFunc* f, int nargs, const char* where, int line) {\n    int total',
            'static void vm_check_arity(const PxVMFunc* f, int nargs, const char* where, int line) {\n'
            '    if (1) { (void)f; (void)nargs; (void)where; (void)line; return; }   /* M301-NEG-A */\n    int total',
            'negA')
    wr(VM, s)
    print('M301-NEG-A-ON')

elif CMD == 'negB_on':
    # 负控 B：停用 **C 轨**函数体（`cg_gen_func_named`）发射的实参个数检查（下界 + 上界）。
    #   ⚠️ 实现方式 = **整行注释掉** —— 首版写成 `if (0) s += "…"` 是**普贤语法错误**
    #   （缩进语法没有单行 if 体）⇒ 编译器构建失败、负控「构建失败」而非「判红」，
    #   红得指不到真因（本门首跑实测）。
    #   ⚠️ 9e60d9c（M301 收尾）起，下界的发射被**包进 `if req_n > 0:` 里**（required==0
    #   时省掉恒不成立的下界检查）。此时只注释「发射行」会让该 `if` **体空** ——
    #   空块同样是**普贤语法错误** ⇒ 构建失败（全量门实测：负控 B「构建失败」）。
    #   ⇒ 必须**连同 `if req_n > 0:` 这一行一起注释**，让整个块消失、不留空体。
    s = rd(CG)
    s = one(s, '    if req_n > 0:\n        s += "    if (nargs < ',
            '    # M301-NEG-B(mute): if req_n > 0:\n'
            '        # M301-NEG-B(mute): s += "    if (nargs < ', 'negB-if-and-lt')
    s = one(s, '    s += "    if (nargs > " + str(len(params))',
            '    # M301-NEG-B(mute): s += "    if (nargs > " + str(len(params))', 'negB-gt')
    wr(CG, s)
    print('M301-NEG-B-ON')

elif CMD == 'restore':
    for f in FILES:
        shutil.copy2(os.path.join(SNAP, os.path.basename(f)), f)
    print('M301-NEG-RESTORE-OK')

elif CMD == 'check':
    bad = 0
    for f in FILES:
        a = open(os.path.join(SNAP, os.path.basename(f)), 'rb').read()
        b = open(f, 'rb').read()
        if a != b:
            print('❌ 未还原: %s' % f)
            bad += 1
    if bad:
        sys.exit(1)
    print('M301-NEG-RESTORED-OK %d 件逐字节一致' % len(FILES))

else:
    print('用法: negctl.py {snapshot|negA_on|negB_on|restore|check}')
    sys.exit(2)
