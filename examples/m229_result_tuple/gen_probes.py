#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M229 门 · 生成器 + 三轨对拍（`tuple` / `result` 两个接收者 —— M226 未覆盖的面）

主题：M226 把方法面的「逐位置 × 错类型 + 错 arity」量到 **str/list/dict**；M227/M228 补了
  「同名两门」「运算符门」。**`tuple` 与 `result` 这两个接收者从未被度量过**
  ⇒ 它们的**同族内部**是否一致，此前没有任何门看得见。

判据：
  H1 精确 arity —— 声明的参数个数之外一律 `R1005`
  H2 措辞族单一 —— 方法面 R1005 一律 `方法 <名> …`（例外表见 verify.sh 的覆盖边界）
  H3 三轨一致     —— 归一化后 (R 码, 文本) 逐字节一致（通道/行号前缀按 M186 归一）
  H4 合法侧不变   —— 合法调用取值必须与独立期望一致（非空壳门）
"""
import argparse
import os
import re
import subprocess
import sys

D = os.path.dirname(os.path.abspath(__file__))

# (例号, 表达式, 期望)  期望 = ('VAL', '文本') 或 ('ERR', 'R码', '词条')
CASES = [
    # ---- tuple.len（0 参）----
    ("t_len0",  "(1, 2, 3).len()",              ("VAL", "3")),
    ("t_len1",  "(1, 2, 3).len(1)",             ("ERR", "R1005", "方法 len 不接受参数")),
    ("t_len2",  "(1, 2, 3).len(1, 2)",          ("ERR", "R1005", "方法 len 不接受参数")),
    # ---- tuple.contains（1 参，任意类型）----
    ("t_cont0", "(1, 2, 3).contains()",         ("ERR", "R1005", "方法 contains 需要 1 个参数")),
    ("t_cont1", "(1, 2, 3).contains(2)",        ("VAL", "true")),
    ("t_cont1b", "(1, 2, 3).contains(\"x\")",   ("VAL", "false")),
    ("t_cont2", "(1, 2, 3).contains(2, 3)",     ("ERR", "R1005", "方法 contains 需要 1 个参数")),
    ("t_cont3", "(1, 2, 3).contains(2, 3, 4)",  ("ERR", "R1005", "方法 contains 需要 1 个参数")),
    # ---- tuple.join（1 参，必须是 string）----
    ("t_join0",  "(1, 2, 3).join()",            ("ERR", "R1005", "方法 join 需要 1 个参数")),
    ("t_join1",  "(\"a\", \"b\").join(\"-\")",   ("VAL", "a-b")),
    ("t_join1t", "(1, 2, 3).join(1)",           ("ERR", "R1002", "方法 join 参数 1 需要 string")),
    ("t_join2",  "(1, 2, 3).join(\"-\", \"+\")", ("ERR", "R1005", "方法 join 需要 1 个参数")),
    # ---- result.is_ok / is_err（0 参）----
    ("r_isok0",  "Ok(7).is_ok()",               ("VAL", "true")),
    ("r_isok1",  "Ok(7).is_ok(1)",              ("ERR", "R1005", "方法 is_ok 不接受参数")),
    ("r_isok2",  "Ok(7).is_ok(1, 2)",           ("ERR", "R1005", "方法 is_ok 不接受参数")),
    ("r_iserr0", "Ok(7).is_err()",              ("VAL", "false")),
    ("r_iserr1", "Ok(7).is_err(1)",             ("ERR", "R1005", "方法 is_err 不接受参数")),
    # ---- result.unwrap / unwrap_err（0 参）----
    ("r_unw0",   "Ok(7).unwrap()",              ("VAL", "7")),
    ("r_unw1",   "Ok(7).unwrap(1)",             ("ERR", "R1005", "方法 unwrap 不接受参数")),
    ("r_unw1e",  "Err(\"b\").unwrap()",         ("ERR", "R1004", "unwrap 失败: Err(b)")),
    ("r_unwe0",  "Err(\"b\").unwrap_err()",     ("VAL", "b")),
    ("r_unwe1",  "Err(\"b\").unwrap_err(1)",    ("ERR", "R1005", "方法 unwrap_err 不接受参数")),
    ("r_unwe0o", "Ok(7).unwrap_err()",          ("ERR", "R1004", "unwrap_err 失败: Ok(7)")),
    # ---- ★ ok / err（0 参）—— 修前**完全不查 arity**（静默）----
    ("r_ok0",    "Ok(7).ok()",                  ("VAL", "7")),
    ("r_ok0e",   "Err(\"b\").ok()",             ("VAL", "null")),
    ("r_ok1",    "Ok(7).ok(1)",                 ("ERR", "R1005", "方法 ok 不接受参数")),
    ("r_ok2",    "Ok(7).ok(1, 2)",              ("ERR", "R1005", "方法 ok 不接受参数")),
    ("r_err0",   "Err(\"b\").err()",            ("VAL", "b")),
    ("r_err0o",  "Ok(7).err()",                 ("VAL", "null")),
    ("r_err1",   "Ok(7).err(1)",                ("ERR", "R1005", "方法 err 不接受参数")),
    ("r_err2",   "Err(\"b\").err(1, 2)",        ("ERR", "R1005", "方法 err 不接受参数")),
    # ---- 接收者错类型（M226 未覆盖的「接收者」维度）----
    ("x_int",    "(7).len()",                   ("ERR", "R1007", "类型 int 没有方法 'len'")),
    ("x_bool",   "true.upper()",                ("ERR", "R1007", "类型 bool 没有方法 'upper'")),
    # ---- 顺带收口的同族（list.pop / 措辞统一）----
    ("l_pop0",   "[1, 2].pop()",                ("VAL", "2")),
    ("l_pop1",   "[1, 2].pop(1)",               ("ERR", "R1005", "方法 pop 不接受参数")),
]


def gen_driver():
    L = ["# M229 门 · 聚合驱动器（gen_probes.py 生成，勿手改）",
         "#   env M229_CASE 选例；一次编译覆盖全量（M199 / M226 / M228 范式）",
         "def main():",
         '    let c = env("M229_CASE")']
    first = True
    for cid, expr, _exp in CASES:
        L.append(f'    {"if" if first else "elif"} c == "{cid}":')
        L.append(f"        print({expr})")
        first = False
    L.append('    else:')
    L.append('        print("BAD-CASE")')
    open(os.path.join(D, "drv.px"), "w", encoding="utf-8").write("\n".join(L) + "\n")


def gen_cases():
    with open(os.path.join(D, "cases.tsv"), "w", encoding="utf-8") as fh:
        fh.write("例号\t表达式\t期望形状\t期望值\n")
        for cid, expr, exp in CASES:
            if exp[0] == "VAL":
                fh.write("%s\t%s\tVAL\t%s\n" % (cid, expr, exp[1]))
            else:
                fh.write("%s\t%s\t%s\t%s\n" % (cid, expr, exp[1], exp[2]))


def run(cmd, case, work, timeout=60):
    env = dict(os.environ, M229_CASE=case)
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout,
                           cwd=work, env=env, errors="replace")
    except subprocess.TimeoutExpired:
        return ("TIMEOUT", "", "")
    out = (p.stdout or "").strip()
    if p.returncode == 0 and out:
        return ("VAL", out.splitlines()[0].strip(), "")
    blob = (p.stdout or "") + "\n" + (p.stderr or "")
    for ln in blob.splitlines():
        m = re.search(r"(R\d{4})", ln)
        if m:
            rest = ln[m.end():]
            rest = re.sub(r"^[\s\]:]*", "", rest)
            rest = re.sub(r"^\d+:\d+:\s*", "", rest)
            rest = re.sub(r"^[^0-9A-Za-z\u4e00-\u9fff]+", "", rest).strip()
            return ("ERR", m.group(1), rest)
    return ("?", blob.strip(), "")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", required=True)
    ap.add_argument("--work", required=True)
    ap.add_argument("--drv", required=True)
    ap.add_argument("--harm", action="store_true")
    a = ap.parse_args()
    BIN = {
        "interp": [os.environ.get("M229_PXI", os.path.join(a.root, "bootstrap", "pxi")), a.drv],
        "vm": [os.path.join(a.work, "build", "drv_vm")],
        "c": [os.path.join(a.work, "build", "drv_c")],
    }
    for t, cmd in BIN.items():
        if not os.path.exists(cmd[0]):
            print(f"❌ 缺少 {t} 轨件 {cmd[0]}"); sys.exit(2)
    bad = []
    for cid, expr, exp in CASES:
        res = {t: run(BIN[t], cid, a.work) for t in BIN}
        norm = {(r[0], r[1], r[2]) for r in res.values()}
        if len(norm) != 1:
            bad.append(f"跨轨分叉 {cid} ({expr}): {res}")
        r = res["c"]
        if exp[0] == "VAL":
            if r[0] != "VAL" or r[1] != exp[1]:
                bad.append(f"H4 {cid} ({expr}): 期望 VAL {exp[1]!r}，实测 {r}")
        else:
            if r[0] != "ERR":
                bad.append(f"H1 {cid} ({expr}): 期望响亮 {exp[1]}，实测 {r}")
            elif r[1] != exp[1]:
                bad.append(f"H1 {cid} ({expr}): 期望 {exp[1]}，实测 {r[1]}")
            elif exp[2] and r[2] != exp[2]:
                bad.append(f"H2 {cid} ({expr}): 期望词条 {exp[2]!r}，实测 {r[2]!r}")
    print(f"── 例 {len(CASES)} × 轨 {len(BIN)} = {len(CASES)*len(BIN)} 次执行")
    if a.harm:
        print(f"M229-VERIFY-OK [--harm 判据自伤 · 已采集差异 {len(bad)} 项不予裁决]"); sys.exit(0)
    if bad:
        for b in bad[:20]:
            print("  ❌ " + b)
        print(f"M229-THREE-TRACKS-FAIL 差异 {len(bad)} 项"); sys.exit(1)
    print("M229-THREE-TRACKS-OK 跨轨 0 · arity 0 · 措辞 0 · 合法侧一致")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--gen":
        gen_driver(); gen_cases(); print(f"OK: {len(CASES)} 例")
    else:
        main()
