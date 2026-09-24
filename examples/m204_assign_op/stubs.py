#!/usr/bin/env python3
# ============================================================
# M204 门的**打桩器**（与 m202/m203 同款：一处一桩、锚点找不到即 rc=3）
#   缺陷 245：C 轨的**复合赋值**在 Index / Field 目标上静默丢弃运算符
#   缺陷 246：复合赋值到**不存在的成员**时解释轨与编译两轨不同码不同文
# 桩号：
#   A Index 复合赋值退回旧形态（无条件 `px_index_set(o, i, rhs)`）
#   B Field 复合赋值退回旧形态
#   C **顺序违规**：把右值提到目标对象/键**之前**求（M165 要求词法左→右）
#   D **重复求值**：不落临时，直接对 `o`/`i` 求值两次
#   E 解释轨缺陷 246 退回（缺失成员不再报 R1008，回去拿 null 硬算）
# ============================================================
import sys, os

ROOT = sys.argv[2] if len(sys.argv) > 2 else "/data/code/puxian"
CGSTMT = os.path.join(ROOT, "selfhost/cg_stmt.px")
ISTMT = os.path.join(ROOT, "selfhost/istmt.px")

# ── A：Index 复合赋值退回旧形态 ──
A_OLD = '''            if op != "Assign":
                # M204（缺陷 245）：同上（`m[0] += 10` 修前实测 = 赋值 10 ⇒ 静默错值）。
                #   对象与键各落 `_sN`（各求值**恰一次**），顺序守 M165（目标对象 → 键 → 右值）。
                let to = cg_seq_new()
                let ti = cg_seq_new()
                let full = cg_assign_op_local(op, "px_index(" + to + ", " + ti + ")", rhs)
                return pad + "({ LXValue " + to + " = " + o + "; LXValue " + ti + " = " + i + "; px_index_set(" + to + ", " + ti + ", " + full + "); });\\n"'''
A_NEW = '''            if op != "Assign":
                # NEGCTL-A：退回旧形态（op 被丢弃）
                let sj = cg_seq_join([tgt[1], tgt[2], stmt[3]], [o, i, rhs])
                return pad + cg_seq_wrap(sj[0], "px_index_set(" + sj[1][0] + ", " + sj[1][1] + ", " + sj[1][2] + ")") + ";\\n"'''

# ── B：Field 复合赋值退回旧形态 ──
B_OLD = '''            if op != "Assign":
                # M204（缺陷 245）：复合赋值**必须用上 op** —— 修前这里**无条件**发射
                #   `px_field_set(o, "f", rhs)`，`op` 被**静默丢弃**（`p.x += 10` 实测 = 赋值 10）。
                #   目标对象先落 `_sN`（求值**恰一次**），旧值 = px_field(_sN, "f")，
                #   新值 = <op>(旧值, rhs)；顺序守 M165（目标对象 → 右值）。
                let to = cg_seq_new()
                let full = cg_assign_op_local(op, "px_field(" + to + ", \\"" + fname + "\\")", rhs)
                return pad + "({ LXValue " + to + " = " + o + "; px_field_set(" + to + ", \\"" + fname + "\\", " + full + "); });\\n"'''
B_NEW = '''            if op != "Assign":
                # NEGCTL-B：退回旧形态（op 被丢弃）
                let sj = cg_seq_join([tgt[1], stmt[3]], [o, rhs])
                return pad + cg_seq_wrap(sj[0], "px_field_set(" + sj[1][0] + ", \\"" + fname + "\\", " + sj[1][1] + ")") + ";\\n"'''

# ── C：顺序违规（右值先于目标对象/键求值）──
C_OLD = '''                let to = cg_seq_new()
                let ti = cg_seq_new()
                let full = cg_assign_op_local(op, "px_index(" + to + ", " + ti + ")", rhs)
                return pad + "({ LXValue " + to + " = " + o + "; LXValue " + ti + " = " + i + "; px_index_set(" + to + ", " + ti + ", " + full + "); });\\n"'''
C_NEW = '''                let to = cg_seq_new()
                let ti = cg_seq_new()
                let tv = cg_seq_new()
                let full = cg_assign_op_local(op, "px_index(" + to + ", " + ti + ")", tv)
                return pad + "({ LXValue " + tv + " = " + rhs + "; LXValue " + to + " = " + o + "; LXValue " + ti + " = " + i + "; px_index_set(" + to + ", " + ti + ", " + full + "); });\\n"'''

# ── D：重复求值（不落临时；目标对象/键各被求值两次）──
D_OLD = C_OLD
D_NEW = '''                let full = cg_assign_op_local(op, "px_index(" + o + ", " + i + ")", rhs)
                return pad + "({ px_index_set(" + o + ", " + i + ", " + full + "); });\\n"'''

# ── E：解释轨 246 退回（缺键/缺字段不再报 R1008）──
E_OLD = '''                if not ov.has(iv):
                    return Err(i_r1008("字典没有键 '" + iv + "'", pos))
                let old = ov[iv]'''
E_NEW = '''                var old = null
                if ov.has(iv):
                    old = ov[iv]'''

SPEC = {
    "A": (CGSTMT, A_OLD, A_NEW),
    "B": (CGSTMT, B_OLD, B_NEW),
    "C": (CGSTMT, C_OLD, C_NEW),
    "D": (CGSTMT, D_OLD, D_NEW),
    "E": (ISTMT, E_OLD, E_NEW),
}


def main():
    if len(sys.argv) < 2:
        print("用法：python3 stubs.py <A|B|C|D|E> [仓库根]", file=sys.stderr)
        return 2
    letter = sys.argv[1]
    if letter not in SPEC:
        print(f"未知桩号 {letter}", file=sys.stderr)
        return 2
    path, old, new = SPEC[letter]
    s = open(path, encoding="utf-8").read()
    n = s.count(old)
    if n != 1:
        print(f"NEGCTL-{letter} 打桩失败：锚点在 {path} 出现 {n} 次（期望 1）", file=sys.stderr)
        return 3
    open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
    print(f"NEGCTL-{letter} 已打桩于 {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
