#!/usr/bin/env python3
# ============================================================
# M253 负控：**忠实撤回**本轮两处修复 ⇒ 期望值层必须变红。
#   用法：
#     negctl.py --save    快照将要改的源文件（写到 <work>/neg/<name>）
#     negctl.py --patch A 撤回缺陷 A 的修复（AES-GCM 空明文）
#     negctl.py --patch B 撤回缺陷 B 的修复（go_errno_string int 截断）
#     negctl.py --restore 从快照**逐字节**还原
#     negctl.py --selftest 自证：每条替换串在**当前源码**里命中恰好 1 次
#   ⚠️ 撤回的是「修复」，不是「实现」—— 即把代码退回**修前原文**，用于证明门有牙。
# ============================================================
import argparse, os, shutil, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='/data/code/puxian')
ap.add_argument('--work', required=True)
ap.add_argument('--save', action='store_true')
ap.add_argument('--patch', choices=['A', 'B'])
ap.add_argument('--restore', action='store_true')
ap.add_argument('--selftest', action='store_true')
a = ap.parse_args()

FILES = ['runtime/runtime_aes.c', 'runtime/runtime.c']
SNAP = os.path.join(a.work, 'neg')

# ── 缺陷 A：AES-GCM 解密拒空明文 ──
A_NEW_1 = ('    // M253（缺陷 444）：GCM 的密文长度域是 [0, ∞) —— 空明文 ⇒ 恰好 16 字节 tag。\n'
           '    //   修前判据 `< 17`（= 至少 1 字节密文）⇒ **自己加密的 16 字节自己解不开**，\n'
           '    //   且与 Go crypto/aes-gcm 在空明文上不互通（aead.Seal(pt=[]) 恰好 16 字节）。\n'
           '    if (alllen < 16) { free(all); return px_null(); }\n')
A_OLD_1 = '    if (alllen < 17) { free(all); return px_null(); }\n'
A_NEW_2 = ('    // M253（缺陷 444）：同 A-1 —— 密文可为 0 字节（空明文），只需 ≥16 字节 tag。\n'
           '    if (ctlen < 16) return px_null();\n')
A_OLD_2 = '    if (ctlen < 17) return px_null();  // 至少 1 字节密文 + 16 字节 tag\n'

# ── 缺陷 B：go_errno_string int 截断 ──
B_NEW_1 = ('    // M253（缺陷 445）：`(int)` 截断 ⇒ 2^32+1 被截成 1 ⇒ 回「operation not permitted」\n'
           '    //   （用户从未问过的 errno）；2^31 回「errno -2147483648」。全链路 int64_t。\n'
           '    int64_t n = px_arg_int(args[0], "go_errno_string", "errno");\n'
           '    if (n >= 0 && n < (int64_t)GO_ERRNO_STR_N && GO_ERRNO_STR[n][0] != \'\\0\') return px_str(GO_ERRNO_STR[n]);\n'
           '    char buf[32];\n'
           '    snprintf(buf, sizeof(buf), "errno %lld", (long long)n);\n')
B_OLD_1 = ('    int n = (int)px_arg_int(args[0], "go_errno_string", "errno");\n'
           '    if (n >= 0 && n < GO_ERRNO_STR_N && GO_ERRNO_STR[n][0] != \'\\0\') return px_str(GO_ERRNO_STR[n]);\n'
           '    char buf[32];\n'
           '    snprintf(buf, sizeof(buf), "errno %d", n);\n')
B_NEW_2 = ('// M253（缺陷 445）：与 bi_go_errno_string 同一规则的第二处实现 ⇒ 同改 int64_t\n'
           '//   （M230 的教训：一条语义分落两处 ⇒ 只收口一处就是结构性漂移）。\n'
           'static void px_go_errno_into(char* buf, int cap, int64_t n) {\n'
           '    if (!buf || cap <= 0) return;\n'
           '    if (n >= 0 && n < (int64_t)GO_ERRNO_STR_N && GO_ERRNO_STR[n][0] != \'\\0\') snprintf(buf, (size_t)cap, "%s", GO_ERRNO_STR[n]);\n'
           '    else snprintf(buf, (size_t)cap, "errno %lld", (long long)n);\n'
           '}\n')
B_OLD_2 = ('static void px_go_errno_into(char* buf, int cap, int n) {\n'
           '    if (!buf || cap <= 0) return;\n'
           '    if (n >= 0 && n < GO_ERRNO_STR_N && GO_ERRNO_STR[n][0] != \'\\0\') snprintf(buf, (size_t)cap, "%s", GO_ERRNO_STR[n]);\n'
           '    else snprintf(buf, (size_t)cap, "errno %d", n);\n'
           '}\n')

EDITS = {
    'A': [('runtime/runtime_aes.c', A_NEW_1, A_OLD_1), ('runtime/runtime_aes.c', A_NEW_2, A_OLD_2)],
    'B': [('runtime/runtime.c', B_NEW_1, B_OLD_1), ('runtime/runtime.c', B_NEW_2, B_OLD_2)],
}


def read(p):
    return open(os.path.join(a.root, p), encoding='utf-8').read()


def main():
    if a.selftest:
        bad = 0
        for k, edits in EDITS.items():
            for i, (path, new, old) in enumerate(edits, 1):
                s = read(path)
                cn, co = s.count(new), s.count(old)
                ok = (cn == 1 and co == 0)
                print(f'{"✅" if ok else "❌"} NC-{k}#{i} {path}: 修复串 ×{cn} · 修前串 ×{co}')
                bad += 0 if ok else 1
        print(f'--- 自证 {"OK" if not bad else f"FAIL({bad})"} ---')
        return 0 if not bad else 3

    if a.save:
        os.makedirs(SNAP, exist_ok=True)
        for p in FILES:
            dst = os.path.join(SNAP, p.replace('/', '__'))
            shutil.copyfile(os.path.join(a.root, p), dst)
            print(f'✅ 快照 {p} → {dst}')
        return 0

    if a.restore:
        n = 0
        for p in FILES:
            src = os.path.join(SNAP, p.replace('/', '__'))
            if os.path.exists(src):
                open(os.path.join(a.root, p), 'w', encoding='utf-8').write(open(src, encoding='utf-8').read())
                n += 1
        print(f'✅ 已还原 {n} 个文件')
        return 0

    if a.patch:
        # 每处替换必须**精确命中 1 次**，否则整体拒绝（M247/M253 纪律）
        for i, (path, new, old) in enumerate(EDITS[a.patch], 1):
            s = read(path)
            if s.count(new) != 1:
                print(f'❌ NC-{a.patch}#{i} 修复串命中 {s.count(new)} 次 ⇒ 拒绝')
                return 3
        for i, (path, new, old) in enumerate(EDITS[a.patch], 1):
            s = read(path)
            open(os.path.join(a.root, path), 'w', encoding='utf-8').write(s.replace(new, old, 1))
            print(f'✅ NC-{a.patch}#{i} 已撤回 {path}')
        return 0
    return 0


sys.exit(main())
