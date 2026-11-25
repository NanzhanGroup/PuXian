#!/usr/bin/env python3
# ============================================================
# M445 试点 · m177_builtin_parity 的**外部定标**真值生成器
# ------------------------------------------------------------
# 病根（与 m439 同型）：用例 A 的 20 条内置面矩阵过去只做「三轨逐字节一致」，
#   期望值仅 4 条定点 `grep`，且那 4 条是修好后**照输出抄的**。
#   若三轨一起做错（缺陷 920 就是这么漏出去的），门仍然绿。
#
# 口径：把每条用例的期望值改成「由**另一个实现**独立算出」——
#   这里用 CPython 的 min/max/sum/str/len/list 语义（CPython 是这些内置的事实权威，
#   且 PuXian 的这批内置语义本来就是对标 Python）。
#   算不出来的（px 特有规约）必须显式登记 source=px-spec 并写明理由，不许含糊过去。
#
# 输出：TSV 三列 `N<TAB>期望输出行<TAB>来源`，供 check 与用例 A 的 stdout 逐行比对。
# 用法：python3 truth.py > builtins.oracle.tsv
# 非零退出：任何一条「本该能算却算出意外」即中止（不产出半张表）。
# ============================================================
import sys

rows = []          # (n, expected_line, source, note)


def add(n, line, source, note=""):
    rows.append((n, line, source, note))


# ---- 1 dict()：类型名 + 空度。CPython 的类型名就是 'dict'，与 px 的 type() 字符串一致。
add(1, "dict=%s len=%d" % (type({}).__name__, len({})), "py")

# ---- 2 unique：保首次出现顺序（CPython dict.fromkeys 的语义）
add(2, "unique=%s" % list(dict.fromkeys([3, 1, 3, 2, 1])), "py")

# ---- 3 flatten：一层摊平，空内层不产出
add(3, "flatten=%s" % [x for sub in [[1, 2], [3], []] for x in sub], "py")

# ---- 4/5/6 sum：list / tuple / generator
add(4, "sum_list=%d" % sum([1, 2, 3]), "py")
add(5, "sum_tuple=%d" % sum((1, 2, 3)), "py")
add(6, "sum_gen=%d" % sum(x * 2 for x in [1, 2, 3]), "py")

# ---- 7/8/9 min
add(7, "min_list=%s" % min([3, 1, 2]), "py")
add(8, "min_tuple=%s" % min((3, 1, 2)), "py")
add(9, "min_gen=%s" % min(x for x in [3, 1, 2]), "py")

# ---- 10/11/12 max
add(10, "max_list=%s" % max([3, 1, 2]), "py")
add(11, "max_tuple=%s" % max((3, 1, 2)), "py")
add(12, "max_gen=%s" % max(x for x in [3, 1, 2]), "py")

# ---- 13/14 variadic
add(13, "min_multi=%s" % min(3, 7, 5), "py")
add(14, "max_multi=%s" % max(3, 7, 5), "py")

# ---- 15 min("b")：单字符 str 本身就是可迭代，取唯一元素
add(15, "min_str_single=%s" % min("b"), "py")

# ---- 16 min(9)：CPython 会 TypeError('int' object is not iterable) ⇒ 无外部参照。
#      px 的规约（M177 决定）：单标量**原样返回**，不报错。这条只能按 px 规约定标，
#      所以必须显式标 px-spec，且回归风险由「三轨一致」与门 [1] 的 rc=0 兜底。
add(16, "min_one_scalar=9", "px-spec",
    "CPython min(9) 抛 TypeError；px 规约=单标量原样返回（M177 修复时三轨统一的契约）")

# ---- 17 unique 对生成器
add(17, "unique_gen=%s" % list(dict.fromkeys(x % 3 for x in [0, 1, 2, 3, 4])), "py")

# ---- 18 flatten 对 tuple-of-lists
add(18, "flatten_tuple=%s" % [x for sub in ([1], [2, 3]) for x in sub], "py")

# ---- 19 min 对 str 列表（字典序）
add(19, "min_str_iter=%s" % min(["b", "a", "c"]), "py")

# ---- 20 max 对 float 列表
add(20, "max_float=%s" % max([1.5, 2.5, 0.5]), "py")

# ---- 自检：必须恰好 20 条、编号 1..20 连续、无重复
nums = [r[0] for r in rows]
if nums != list(range(1, 21)):
    sys.stderr.write("❌ 用例编号不是 1..20 连续：%r\n" % (nums,))
    sys.exit(3)
py_n = sum(1 for r in rows if r[2] == "py")
spec_n = sum(1 for r in rows if r[2] == "px-spec")
if py_n + spec_n != 20:
    sys.stderr.write("❌ source 列只许 py / px-spec\n")
    sys.exit(3)

out = []
out.append("# M445 外部定标 · m177_builtin_parity 用例 A（20 条内置面矩阵）")
out.append("# 生成器：examples/m177_builtin_parity/oracle/truth.py（CPython 独立实现，非 px 输出）")
out.append("# 格式：N<TAB>期望输出行<TAB>来源(py|px-spec)")
out.append("# py = 由 CPython 语义独立算出；px-spec = px 特有规约（无外部参照，必须写明理由）")
out.append("# py=%d 条 · px-spec=%d 条" % (py_n, spec_n))
for n, line, source, note in rows:
    if note:
        out.append("# note[%d] %s" % (n, note))
    out.append("%d\t%s\t%s" % (n, line, source))
sys.stdout.write("\n".join(out) + "\n")
