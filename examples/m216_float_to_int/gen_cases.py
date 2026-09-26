#!/usr/bin/env python3
# ============================================================
# M216 语料生成器（**单一事实源**）—— 浮点→int 转换族
# ------------------------------------------------------------
# 产出：
#   cases.px     —— 驱动器（按 args()[1] 分派；**一次编译**覆盖全量用例，见 M199 教训）
#   CASES.tsv    —— label / kind / 期望（ok ⇒ 期望 stdout；rej ⇒ R####|消息体）
# 真值来源（**独立第三份**）：Python 的 `int(f)`（向零截断）与 `//`（floor）语义，
#   与 PuXian 的编译实现**不同构**；拒绝侧的「越界/非有限」判据按 C11 6.3.1.4 与
#   IEEE-754 由 Python 侧显式计算（见 EXPECT_* 表）。
# 漂移检测：verify.sh 第 1 层重跑本脚本并与入库的 cases.px / CASES.tsv 逐字节比。
# ============================================================
import argparse, os, sys

# ---- 合法侧：body 是 .px 源码；exp 由 Python 独立算出 ----
LEGAL = [
    ("ok_int_1e18", "print(int(1e18))", str(int(1e18))),
    ("ok_int_neg1e18", "print(int(-1e18))", str(int(-1e18))),
    ("ok_int_trunc", "print(int(2.9), int(-2.9), int(0.5), int(-0.5))",
     " ".join(str(int(x)) for x in (2.9, -2.9, 0.5, -0.5))),
    # 上/下界的**良定义**端点：−2^63 可精确表示；最大可表示且 < 2^63 的 double = 2^63−1024
    ("ok_int_min_double", "print(int(-9223372036854775808.0))",
     str(int(-9223372036854775808.0))),
    ("ok_int_max_double", "print(int(9223372036854774784.0))",
     str(int(9223372036854774784.0))),
    ("ok_idiv_float", "print(7.5 // 2, -7.5 // 2, 0.5 // 1)",
     " ".join(str(x) for x in (int(7.5 // 2), int(-7.5 // 2), int(0.5 // 1)))),
    ("ok_slice_int", "l = [1, 2, 3, 4, 5]\nprint(l[1:3], l[-2:], l[0:2])",
     "[2, 3] [4, 5] [1, 2]"),
    ("ok_bslice_int", "print(len(bytes_slice(bytes(\"abcdef\"), 1, 3)))", "2"),
    ("ok_int_str", "print(int(\"123\"), int(\"-45\"))", "123 -45"),
]

# ---- 拒绝侧：期望 (R码, 消息体)。消息体**不含**轨道前缀与行列号 ----
REJ = [
    ("rej_int_1e30", "print(int(1e30))", "R1003"),
    ("rej_int_neg1e30", "print(int(-1e30))", "R1003"),
    ("rej_int_2p63", "print(int(9223372036854775808.0))", "R1003"),
    ("rej_int_rt_max", "x = 9223372036854775807\nprint(int(float(x)))", "R1003"),
    ("rej_int_nan", "z = 0.0\nprint(int(z / z))", "R1003"),
    ("rej_int_inf", "z = 0.0\no = 1.0\nprint(int(o / z))", "R1003"),
    ("rej_int_ninf", "z = 0.0\nprint(int((0.0 - 1.0) / z))", "R1003"),
    ("rej_idiv_over", "print(1e30 // 1.0)", "R1003"),
    ("rej_idiv_nover", "print(-1e30 // 1.0)", "R1003"),
    ("rej_slice_hi", "l = [1, 2, 3]\nprint(l[0:1e30])", "R1002"),
    ("rej_slice_lo", "l = [1, 2, 3]\nprint(l[1e30:2])", "R1002"),
    ("rej_slice_step", "l = [1, 2, 3]\nprint(l[0:2:1.5])", "R1002"),
    ("rej_bslice", "print(len(bytes_slice(bytes(\"abcdef\"), 1e30, 2)))", "R1002"),
]

# 「切片界错类型」的统一消息（三轨必须逐字节相同，见 runtime 的 px_req_slice_idx
#   与 selfhost/ival.px 的 i_slice 守卫）
SLICE_MSG = "切片索引必须是整数，实际是 float"


def gen_px():
    L = []
    L.append("# ⚠️ 本文件由 gen_cases.py 生成（单一事实源）—— 手改会被第 1 层的漂移检测判红")
    L.append("# M216 驱动器：按 args()[1] 分派（一次编译覆盖全量用例）")
    L.append("a = args()")
    L.append("sel = \"\"")
    L.append("if len(a) >= 2:")
    L.append("    sel = a[1]")
    L.append("")
    L.append("def run_case(sel):")
    for label, body, _ in LEGAL:
        L.append("    if sel == \"%s\":" % label)
        for line in body.split("\n"):
            L.append("        " + line)
        L.append("        return 0")
    for label, body, _ in REJ:
        L.append("    if sel == \"%s\":" % label)
        for line in body.split("\n"):
            L.append("        " + line)
        L.append("        return 0")
    L.append("    print(\"NO-SUCH-CASE: \" + sel)")
    L.append("    return 2")
    L.append("")
    L.append("exit(run_case(sel))")
    L.append("")
    return "\n".join(L)


def gen_tsv():
    rows = []
    for label, _, exp in LEGAL:
        rows.append("\t".join([label, "ok", exp]))
    for label, _, code in REJ:
        if label.startswith("rej_slice") or label == "rej_bslice":
            rows.append("\t".join([label, "rej", code + "|" + SLICE_MSG]))
        else:
            rows.append("\t".join([label, "rej", code + "|"]))
    return "\n".join(rows) + "\n"


ap = argparse.ArgumentParser()
ap.add_argument("--out", default=".")
a = ap.parse_args()
os.makedirs(a.out, exist_ok=True)
with open(os.path.join(a.out, "cases.px"), "w") as f:
    f.write(gen_px())
with open(os.path.join(a.out, "CASES.tsv"), "w") as f:
    f.write(gen_tsv())
print("生成 %d 例（合法 %d · 拒绝 %d）→ %s/{cases.px,CASES.tsv}"
      % (len(LEGAL) + len(REJ), len(LEGAL), len(REJ), a.out))
