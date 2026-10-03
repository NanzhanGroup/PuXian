#!/usr/bin/env python3
# ============================================================
# M250 门 · 负控补丁器（**忠实撤回** + **反向过头**）
# ------------------------------------------------------------
# 纪律（M220/M226 教训）：负控必须「**忠实撤回**」而不是「另造一个错」——
#   每条 `old` 都是**修前原文**（逐字来自本轮 patch 前的源码）。
#
# 两道负控证明**判据两个方向都有牙**：
#   A · 撤回修复（退回 `(int)` 窄化 + `i * 4 + 4` int32 溢出）
#       ⇒ [3] 边界面必须红（越界下标不再「响亮」）
#   B · **做过头**（把界判据收成「只允许 0 号」）
#       ⇒ [3b] 反向判据（合法域上界仍须可取）与 [4] 独立真值必须红
#          —— 只证明「越界被拒」是不够的：**一律拒绝**同样能让 [3] 变绿。
#
# 用法：negctl.py --apply A|B   ·  negctl.py --restore  ·  negctl.py --selftest
#   快照写在 $M250_SNAP（默认 /tmp/m250_negctl_snap），源逐字节还原。
# ============================================================
import argparse, os, shutil, sys

ROOT = os.environ.get('M250_ROOT', '.')
SNAP = os.environ.get('M250_SNAP', '/tmp/m250_negctl_snap')
RT = 'runtime/runtime_onnx.c'

HELPER = '''static int onnx_at_in_range(int64_t idx, int len, int elem_bytes) {
    return idx >= 0 && idx < (int64_t)(len / elem_bytes);
}'''

A_PAIRS = [
    # ── f32_at ──
    (RT,
     '''    float f;
    int64_t i;
    (void)ctx;
    if (nargs != 2) px_error("R1002: f32_at 需要 2 个参数 (bytes, i)");
    b = onnx_bytes_arg(args[0], &len);
    if (!b || args[1].type != PX_INT) return px_err(px_str("f32_at: 参数类型错误"));
    i = args[1].as.i;
    if (!onnx_at_in_range(i, len, 4)) return px_err(px_str("f32_at: 下标越界"));
    memcpy(&f, b + (size_t)i * 4, 4);''',
     '''    float f;
    int i;
    (void)ctx;
    if (nargs != 2) px_error("R1002: f32_at 需要 2 个参数 (bytes, i)");
    b = onnx_bytes_arg(args[0], &len);
    if (!b || args[1].type != PX_INT) return px_err(px_str("f32_at: 参数类型错误"));
    i = (int)args[1].as.i;
    if (i < 0 || (i * 4 + 4) > len) return px_err(px_str("f32_at: 下标越界"));
    memcpy(&f, b + i * 4, 4);'''),
    # ── i64_at ──
    (RT,
     '''    int64_t v;
    int64_t i;
    (void)ctx;
    if (nargs != 2) px_error("R1002: i64_at 需要 2 个参数 (bytes, i)");
    b = onnx_bytes_arg(args[0], &len);
    if (!b || args[1].type != PX_INT) return px_err(px_str("i64_at: 参数类型错误"));
    i = args[1].as.i;
    if (!onnx_at_in_range(i, len, 8)) return px_err(px_str("i64_at: 下标越界"));
    memcpy(&v, b + (size_t)i * 8, 8);''',
     '''    int64_t v;
    int i;
    (void)ctx;
    if (nargs != 2) px_error("R1002: i64_at 需要 2 个参数 (bytes, i)");
    b = onnx_bytes_arg(args[0], &len);
    if (!b || args[1].type != PX_INT) return px_err(px_str("i64_at: 参数类型错误"));
    i = (int)args[1].as.i;
    if (i < 0 || (i * 8 + 8) > len) return px_err(px_str("i64_at: 下标越界"));
    memcpy(&v, b + i * 8, 8);'''),
]

B_PAIRS = [
    (RT, HELPER,
     '''static int onnx_at_in_range(int64_t idx, int len, int elem_bytes) {
    (void)len; (void)elem_bytes;
    return idx == 0;   /* M250 负控 B：把界判据**做过头**（只允许 0 号）*/
}'''),
]

ap = argparse.ArgumentParser()
ap.add_argument('--apply', choices=['A', 'B'])
ap.add_argument('--restore', action='store_true')
ap.add_argument('--snapshot', action='store_true')
ap.add_argument('--selftest', action='store_true')
a = ap.parse_args()
PAIRS = {'A': A_PAIRS, 'B': B_PAIRS}


def rd(rel):
    return open(os.path.join(ROOT, rel), encoding='utf-8').read()


def wr(rel, s):
    open(os.path.join(ROOT, rel), 'w', encoding='utf-8').write(s)


def snapshot():
    shutil.rmtree(SNAP, ignore_errors=True)
    os.makedirs(SNAP)
    files = sorted({p[0] for p in A_PAIRS + B_PAIRS})
    for f in files:
        d = os.path.join(SNAP, f)
        os.makedirs(os.path.dirname(d), exist_ok=True)
        shutil.copy2(os.path.join(ROOT, f), d)
    open(os.path.join(SNAP, 'MANIFEST'), 'w').write('\n'.join(files) + '\n')
    print(f'✅ 快照 {len(files)} 个文件 → {SNAP}')


def restore():
    mp = os.path.join(SNAP, 'MANIFEST')
    if not os.path.exists(mp):
        print('❌ 无快照'); sys.exit(1)
    n = 0
    for f in open(mp).read().split():
        shutil.copy2(os.path.join(SNAP, f), os.path.join(ROOT, f))
        n += 1
    print(f'✅ 已还原 {n} 个文件（逐字节）')


if a.snapshot:
    snapshot(); sys.exit(0)
if a.restore:
    restore(); sys.exit(0)
if a.selftest:
    bad = 0
    for tag, ps in (('A', A_PAIRS), ('B', B_PAIRS)):
        for rel, cur, old in ps:
            s = rd(rel)
            if s.count(cur) != 1:
                print(f'❌ 负控 {tag}：锚点「已修文本」在 {rel} 里命中 {s.count(cur)} 次（必须恰 1）')
                bad += 1
            if old in s:
                print(f'❌ 负控 {tag}：修前文本**已存在** ⇒ 撤回无意义')
                bad += 1
    print(f'自证：{len(A_PAIRS) + len(B_PAIRS)} 条锚点 · 违例 {bad}')
    sys.exit(0 if bad == 0 else 1)
if not a.apply:
    ap.print_help(); sys.exit(2)

for rel, cur, old in PAIRS[a.apply]:
    s = rd(rel)
    if s.count(cur) != 1:
        print(f'❌ 锚点不唯一（{s.count(cur)} 次）· 拒绝打桩 —— 先跑 --selftest')
        sys.exit(1)
    if old in s:
        print(f'❌ 修前文本已存在 ⇒ 拒绝打桩')
        sys.exit(1)
    wr(rel, s.replace(cur, old))
print(f'✅ 负控 {a.apply} 已打桩（{len(PAIRS[a.apply])} 处）')
