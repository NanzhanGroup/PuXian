#!/usr/bin/env python3
# ============================================================
# M203 门的**打桩器**（与 m202 同款：一处一桩、找不到锚点即 rc=3）
#   缺陷 243：「此类型不支持X / 不可迭代」族的**三轨同码同文**
#   四个站点（M191 只修了「切片」那一个）：
#     A 索引      解释轨 ival.px   `此类型不支持索引`      （缺 `: <t>`）
#     B 迭代      解释轨 ival.px   `此类型不可迭代`        （缺 `: <t>`；编译两轨**借 len**）
#     C 索引赋值  解释轨 istmt.px  `索引赋值目标不支持`    （**整个词条**与编译轨不同）
#     D 迭代长度  编译两轨          `px_len` ⇒ `len 不支持类型 int`（消息指向用户没写过的 len）
# ============================================================
import sys, os

ROOT = sys.argv[2] if len(sys.argv) > 2 else "/data/code/puxian"
RUNTIME = os.path.join(ROOT, "runtime/runtime.c")
IVAL = os.path.join(ROOT, "selfhost/ival.px")
ISTMT = os.path.join(ROOT, "selfhost/istmt.px")

# A：解释轨索引文案退回「无类型」
A_OLD = '''    # M203（缺陷 243-a）：带**实际类型**（编译轨 px_index 一直如此）⇒ 三轨同文
    return Err(i_r1002("此类型不支持索引: " + t, pos))'''
A_NEW = '''    return Err(i_r1002("此类型不支持索引", pos))   # NEGCTL-A'''

# B：解释轨迭代文案退回「无类型」
B_OLD = '''    # M203（缺陷 243-b）：带**实际类型**；编译轨不再借 `len`（见 px_iter_prepare）
    return Err(i_r1002("此类型不可迭代: " + t, pos))'''
B_NEW = '''    return Err(i_r1002("此类型不可迭代", pos))   # NEGCTL-B'''

# C：解释轨索引赋值文案退回旧词条
C_OLD = '''        # M203（缺陷 243-c）：**整个词条**与编译轨不同（编译轨 = 此类型不支持索引赋值: <t>）
        return Err(i_r1002("此类型不支持索引赋值: " + t, pos))'''
C_NEW = '''        return Err(i_r1002("索引赋值目标不支持", pos))   # NEGCTL-C'''

# D：runtime 的迭代入口文案退回「借 len」（= 编译两轨修前形态）
D_OLD = '''        default:
            px_error("R1002: 此类型不可迭代: %s", px_type_name(v));
            return 0;'''
D_NEW = '''        default:
            px_error("R1002: len 不支持类型 %s", px_type_name(v));   /* NEGCTL-D */
            return 0;'''

# E：runtime 的迭代入口**放行 int**（静默 0 次迭代）⇒ 响亮退回静默
E_OLD = '''        case PX_TUPLE:
        case PX_GEN:
            return px_len(v);'''
E_NEW = '''        case PX_TUPLE:
        case PX_GEN:
            return px_len(v);
        case PX_INT:
            return 0;   /* NEGCTL-E：静默 0 次迭代 */'''

SPEC = {
    "A": (IVAL, A_OLD, A_NEW),
    "B": (IVAL, B_OLD, B_NEW),
    "C": (ISTMT, C_OLD, C_NEW),
    "D": (RUNTIME, D_OLD, D_NEW),
    "E": (RUNTIME, E_OLD, E_NEW),
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
