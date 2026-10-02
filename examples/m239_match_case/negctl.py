#!/usr/bin/env python3
# M239 门 · 负控锚点（**忠实撤回**本轮修复的各个面；源逐字节还原）
#   A = VM 轨（bc_emit.px）· B = C 轨（cg_expr.px）· C = 解释轨（iexpr.px + istmt.px）
#   D = 判据自伤（把三轨比对换成恒等 ⇒ A 的红必须消失）
# 用法: negctl.py --root <仓库根> --snap <快照目录> --apply A|B|C|D | --restore
import argparse, os, shutil, sys

AP = argparse.ArgumentParser()
AP.add_argument('--root', required=True)
AP.add_argument('--snap', required=True)
AP.add_argument('--apply')
AP.add_argument('--restore', action='store_true')
a = AP.parse_args()
R = a.root

FILES = ['selfhost/bc_emit.px', 'selfhost/cg_expr.px', 'selfhost/iexpr.px', 'selfhost/istmt.px',
         'examples/m239_match_case/three_tracks.py']

def snap():
    # M239s1（缺陷 394 · 门自身）：**只做首次快照**。
    #   原实现每次 `--apply` 都 `shutil.copy` 覆盖快照 ⇒ 负控 D 段是
    #   「先 `--apply A` 再 `--apply D`」⇒ 第二次 snap 把**已被 A 污染**的源存进快照
    #   ⇒ 之后 `--restore` 恢复的是 **A 的补丁** ⇒ **源码残留**（实测 `bc_emit.px` 留下
    #   `MATCHFAIL→MOV` + 删 `bc_bind_var` + 删 `MATCHTUP` 三处；m116 全量门据此报
    #   「门有副作用」并要求人工 `git checkout`）。快照语义 = 「**进门时的源**」，只能一次。
    os.makedirs(a.snap, exist_ok=True)
    for f in FILES:
        p = os.path.join(R, f)
        s = os.path.join(a.snap, f.replace('/', '__'))
        if os.path.exists(p) and not os.path.exists(s):
            shutil.copy(p, s)

def restore():
    for f in FILES:
        p = os.path.join(R, f); s = os.path.join(a.snap, f.replace('/', '__'))
        if os.path.exists(s):
            shutil.copy(s, p)

def sub(f, old, new, cnt=1):
    p = os.path.join(R, f)
    s = open(p, encoding='utf-8').read()
    n = s.count(old)
    if n != cnt:
        print('❌ 锚点命中 %d 次（期望 %d）: %s :: %s' % (n, cnt, f, old[:70])); sys.exit(2)
    open(p, 'w', encoding='utf-8').write(s.replace(old, new))

PATCH = {
 'A': [
  ('selfhost/bc_emit.px',
   '        bc_bind_var(func, name, t)\n        return\n    if pty == "PatTuple":',
   '        return\n    if pty == "PatTuple":'),
  ('selfhost/bc_emit.px',
   '        bc_emit_inst(func, "MATCHFAIL", 0, t, 0)',
   '        bc_emit_inst(func, "MOV", d, t, 0)'),
  ('selfhost/bc_emit.px',
   '''        let tb = bc_tmp(func)
        bc_emit_inst(func, "MATCHTUP", tb, t, len(items))
        let tj = len(func["bc"])
        bc_emit_inst(func, "JMPF", tb, 0, 0)
        jumps.append(tj)
''', ''),
 ],
 'B': [
  ('selfhost/cg_expr.px',
   '''        let lv = cg_match_bind(name)
        return "(" + lv + " = " + subject + ", 1)"''',
   '        return "1"'),
  ('selfhost/cg_expr.px',
   '        s += "else { px_match_fail(" + t + "); } "',
   '        s += "else { } "'),
  ('selfhost/cg_expr.px',
   '''            if arms[ai][2] != null:
                let g = cg_gen_expr(arms[ai][2])
                full = "(" + cond_c + " && px_is_truthy(" + g + "))"''',
   '''            if false:
                let g = cg_gen_expr(arms[ai][2])
                full = "(" + cond_c + " && px_is_truthy(" + g + "))"'''),
 ],
 'C': [
  ('selfhost/iexpr.px',
   '''                var guard_ok = true
                if arm[2] != null:
                    let gr = i_eval_expr(arm[2], child)
                    if gr.is_err():
                        return Err(gr.err())
                    guard_ok = i_truthy(gr.unwrap())
                if guard_ok:
                    return i_eval_arm_body(arm[3], child)''',
   '''                return i_eval_expr(arm[3], child)'''),
  ('selfhost/istmt.px',
   '''        let ev = r.unwrap()
        if stmt[1][0] == "Match" and type(ev) == "dict" and ev.has("__flow__"):
            return Ok(ev)
        return Ok(null)''',
   '        return Ok(null)'),
 ],
 'D': [
  ('examples/m239_match_case/three_tracks.py',
   '''    if r[0] == r[1] == r[2]:''',
   '''    if True:'''),
 ],
}

if a.restore:
    restore(); print('restore ok'); sys.exit(0)
if not a.apply:
    print('用法: --apply A|B|C|D | --restore'); sys.exit(2)
snap()
for f, old, new in PATCH[a.apply]:
    sub(f, old, new)
print('apply %s ok' % a.apply)
