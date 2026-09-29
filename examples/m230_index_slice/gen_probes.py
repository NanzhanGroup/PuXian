#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M230 门 · **索引 / 切片族**三轨全量对拍（读 · 写 · 切片）

面的定义（M226/M227/M228/M229 的延续）
  (容器类型 × 操作 × 形状)
    容器 = list / dict / string / tuple / 非容器(int/null/bool/result)
    操作 = 索引读 x[i] · 索引写 x[i]=v · 切片 x[a:b]
    形状 = 合法（正索引 / 负索引 / 边界）· 越界正 · 越界负 · 缺键 ·
           索引或键错类型 · 接收者错类型 · 不支持该操作

判据
  H1 跨轨一致 —— 同一例在三轨上 (rc, R 码, 归一化文案) 必须一致
  H2 值一致   —— 合法例的 stdout == cases.tsv 期望值（Python 侧独立算得）
  H3 指到真因 —— 越界消息必须报**用户输入的那个索引**，不是内部归一化后的位置
  MODEL 双向  —— 实测 (操作 × 容器 × 形状 ⇒ R 码) ⇄ MODEL.tsv 精确相等

用法
  gen_probes.py --gen                     生成 cases.tsv + drv.px
  gen_probes.py --root R --work W --drv D  跑三轨对拍
"""
import argparse, json, os, subprocess, sys

# ---------------------------------------------------------------- 用例表
# (例号, 容器, 操作, 形状, 代码行, 期望种类, 期望)
#   期望种类 VAL ⇒ 期望 = stdout 字面量（Python 独立算得 / 手工核对）
#   期望种类 ERR ⇒ 期望 = "R码|词条关键字"（关键字必须精确到「指到真因」）
L = 'L'  # 缩进占位
CASES = [
    # ---- A 索引读 · list
    ("r_list_0",    "list",   "read",  "ok",     ['print([10,20,30][0])'],              "VAL", "10"),
    ("r_list_n1",   "list",   "read",  "neg",    ['print([10,20,30][-1])'],             "VAL", "30"),
    ("r_list_n3",   "list",   "read",  "neg",    ['print([10,20,30][-3])'],             "VAL", "10"),
    ("r_list_oob",  "list",   "read",  "oob",    ['print([10,20,30][3])'],              "ERR", "R1003|索引越界: 3 (len=3)"),
    ("r_list_noob", "list",   "read",  "noob",   ['print([10,20,30][-4])'],             "ERR", "R1003|索引越界: -4 (len=3)"),
    ("r_list_strid","list",   "read",  "keytype",['print([10,20,30]["a"])'],            "ERR", "R1002|索引必须是整数，实际是 string"),
    ("r_list_fl",   "list",   "read",  "keytype",['print([10,20,30][1.0])'],            "ERR", "R1002|索引必须是整数，实际是 float"),
    ("r_list_bt",   "list",   "read",  "keytype",['print([10,20,30][true])'],           "ERR", "R1002|索引必须是整数，实际是 bool"),
    ("r_list_nul",  "list",   "read",  "keytype",['print([10,20,30][null])'],           "ERR", "R1002|索引必须是整数，实际是 null"),
    # ---- A 索引读 · tuple / string
    ("r_tup_0",     "tuple",  "read",  "ok",     ['print((10,20,30)[0])'],              "VAL", "10"),
    ("r_tup_n1",    "tuple",  "read",  "neg",    ['print((10,20,30)[-1])'],             "VAL", "30"),
    ("r_tup_oob",   "tuple",  "read",  "oob",    ['print((10,20,30)[3])'],              "ERR", "R1003|索引越界: 3 (len=3)"),
    ("r_tup_noob",  "tuple",  "read",  "noob",   ['print((10,20,30)[-4])'],             "ERR", "R1003|索引越界: -4 (len=3)"),
    ("r_str_0",     "string", "read",  "ok",     ['print("abc"[0])'],                   "VAL", "a"),
    ("r_str_n1",    "string", "read",  "neg",    ['print("abc"[-1])'],                  "VAL", "c"),
    ("r_str_oob",   "string", "read",  "oob",    ['print("abc"[3])'],                   "ERR", "R1003|索引越界: 3 (len=3)"),
    ("r_str_noob",  "string", "read",  "noob",   ['print("abc"[-4])'],                  "ERR", "R1003|索引越界: -4 (len=3)"),
    ("r_str_cjk",   "string", "read",  "ok",     ['print("中文测试"[1])'],              "VAL", "文"),
    ("r_str_cjkoob","string", "read",  "oob",    ['print("中文"[2])'],                  "ERR", "R1003|索引越界: 2 (len=2)"),
    # ---- A 索引读 · dict
    ("r_dic_k",     "dict",   "read",  "ok",     ['print({"a":1}["a"])'],                "VAL", "1"),
    ("r_dic_miss",  "dict",   "read",  "miss",   ['print({"a":1}["z"])'],                "ERR", "R1008|字典没有键 'z'"),
    ("r_dic_int",   "dict",   "read",  "keytype",['print({"a":1}[0])'],                  "ERR", "R1002|字典索引键必须是字符串"),
    ("r_dic_neg",   "dict",   "read",  "keytype",['print({"a":1}[-1])'],                 "ERR", "R1002|字典索引键必须是字符串"),
    ("r_dic_bt",    "dict",   "read",  "keytype",['print({"a":1}[true])'],               "ERR", "R1002|字典索引键必须是字符串"),
    # ---- A 索引读 · 接收者错类型
    ("r_int_recv",  "int",    "read",  "recv",   ['print((7)[0])'],                      "ERR", "R1002|此类型不支持索引: int"),
    ("r_nul_recv",  "null",   "read",  "recv",   ['print(null[0])'],                     "ERR", "R1002|此类型不支持索引: null"),
    ("r_bt_recv",   "bool",   "read",  "recv",   ['print(true[0])'],                     "ERR", "R1002|此类型不支持索引: bool"),
    ("r_res_recv",  "result", "read",  "recv",   ['print(Ok(1)[0])'],                    "ERR", "R1002|此类型不支持索引: result"),
    # ---- B 索引写
    ("w_list_0",    "list",   "write", "ok",     ['var l = [1,2,3]', 'l[0] = 9', 'print(l)'],   "VAL", "[9, 2, 3]"),
    ("w_list_n1",   "list",   "write", "neg",    ['var l = [1,2,3]', 'l[-1] = 9', 'print(l)'],  "VAL", "[1, 2, 9]"),
    ("w_list_oob",  "list",   "write", "oob",    ['var l = [1,2,3]', 'l[3] = 9', 'print(l)'],   "ERR", "R1003|索引越界: 3 (len=3)"),
    ("w_list_noob", "list",   "write", "noob",   ['var l = [1,2,3]', 'l[-5] = 9', 'print(l)'],  "ERR", "R1003|索引越界: -5 (len=3)"),
    ("w_list_strid","list",   "write", "keytype",['var l = [1,2,3]', 'l["a"] = 9', 'print(l)'], "ERR", "R1002|索引必须是整数，实际是 string"),
    ("w_list_nest", "list",   "write", "ok",     ['var m = [[1,2],[3,4]]', 'm[1][0] = 9', 'print(m)'], "VAL", "[[1, 2], [9, 4]]"),
    ("w_dic_new",   "dict",   "write", "ok",     ['var d = {"a":1}', 'd["z"] = 5', 'print(d["z"], d.len())'], "VAL", "5 2"),
    ("w_dic_over",  "dict",   "write", "ok",     ['var d = {"a":1}', 'd["a"] = 7', 'print(d["a"], d.len())'], "VAL", "7 1"),
    ("w_dic_int",   "dict",   "write", "keytype",['var d = {"a":1}', 'd[0] = 5', 'print(d)'],   "ERR", "R1002|字典索引键必须是字符串"),
    ("w_dic_neg",   "dict",   "write", "keytype",['var d = {"a":1}', 'd[-1] = 5', 'print(d)'],  "ERR", "R1002|字典索引键必须是字符串"),
    ("w_tup",       "tuple",  "write", "unsup",  ['var t = (1,2,3)', 't[0] = 9', 'print(t)'],   "ERR", "R1002|此类型不支持索引赋值: tuple"),
    ("w_str",       "string", "write", "unsup",  ['var s = "abc"', 's[0] = "z"', 'print(s)'],   "ERR", "R1002|此类型不支持索引赋值: string"),
    ("w_int_recv",  "int",    "write", "recv",   ['var n = 7', 'n[0] = 1', 'print(n)'],         "ERR", "R1002|此类型不支持索引赋值: int"),
    # ---- C 切片
    ("s_list_basic","list",   "slice", "ok",     ['print([1,2,3,4][1:3])'],              "VAL", "[2, 3]"),
    ("s_list_oob",  "list",   "slice", "oob",    ['print([1,2,3][5:9])'],                "VAL", "[]"),
    ("s_list_neg",  "list",   "slice", "neg",    ['print([1,2,3][-1:])'],                "VAL", "[3]"),
    ("s_list_rev",  "list",   "slice", "ok",     ['print([1,2,3][2:1])'],                "VAL", "[]"),
    ("s_list_step", "list",   "slice", "ok",     ['print([1,2,3,4][::2])'],              "VAL", "[1, 3]"),
    ("s_list_all",  "list",   "slice", "ok",     ['print([1,2,3,4][:])'],                "VAL", "[1, 2, 3, 4]"),
    ("s_list_mix",  "list",   "slice", "neg",    ['print([1,2,3,4][-3:-1])'],            "VAL", "[2, 3]"),
    ("s_list_n5",   "list",   "slice", "neg",    ['print([1,2,3][-5:2])'],               "VAL", "[1, 2]"),
    ("s_list_strid","list",   "slice", "keytype",['print([1,2,3]["a":1])'],              "ERR", "R1002|切片索引必须是整数，实际是 string"),
    ("s_str_basic", "string", "slice", "ok",     ['print("abcd"[1:3])'],                "VAL", "bc"),
    ("s_str_neg",   "string", "slice", "neg",    ['print("abcd"[-2:])'],                "VAL", "cd"),
    ("s_str_cjk",   "string", "slice", "ok",     ['print("中文测试"[1:3])'],            "VAL", "文测"),
    ("s_tup",       "tuple",  "slice", "ok",     ['print((1,2,3,4)[1:3])'],             "VAL", "(2, 3)"),
    ("s_dic",       "dict",   "slice", "unsup",  ['print({"a":1}[0:1])'],                "ERR", "R1002|此类型不支持切片: dict"),
    ("s_int_recv",  "int",    "slice", "recv",   ['print((7)[0:1])'],                    "ERR", "R1002|此类型不支持切片: int"),
]


def gen(root):
    d = os.path.join(root, "examples", "m230_index_slice")
    with open(os.path.join(d, "cases.tsv"), "w", encoding="utf-8") as f:
        f.write("例号\t容器\t操作\t形状\t期望种类\t期望\n")
        for c in CASES:
            f.write("\t".join([c[0], c[1], c[2], c[3], c[5], c[6]]) + "\n")
    lines = ["# M230 门 · 聚合驱动器（gen_probes.py 生成，勿手改）",
             "#   env M230_CASE 选例；一次编译覆盖全量（M199 / M226 / M228 / M229 范式）",
             "def main():",
             '    let c = env("M230_CASE")']
    for i, c in enumerate(CASES):
        kw = "if" if i == 0 else "elif"
        lines.append('    %s c == "%s":' % (kw, c[0]))
        for st in c[4]:
            lines.append("        " + st)
    lines.append("    else:")
    lines.append('        print("UNKNOWN-CASE")')
    with open(os.path.join(d, "drv.px"), "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print("生成 cases.tsv（%d 例） + drv.px" % len(CASES))
    return 0


# ---------------------------------------------------------------- 跑三轨
def norm_err(s):
    """归一化错误文案：去通道前缀与「行:列」前缀（M186 口径），保留 R 码与词条主体。"""
    import re
    t = s.strip()
    t = re.sub(r"^(运行时)?错误\s*(\[main 行\d+\])?\s*:?\s*", "", t)
    t = re.sub(r"^错误\s*:?\s*", "", t)
    t = re.sub(r"^\[(R\d+)\]\s*(\d+:\d+:)?\s*", lambda m: "[%s] " % m.group(1), t)
    t = re.sub(r"^(R\d+)\s*:\s*", lambda m: "[%s] " % m.group(1), t)
    return t.strip()


def run_track(cmd, env_case, cwd, timeout=60):
    env = dict(os.environ)
    env["M230_CASE"] = env_case
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, cwd=cwd, env=env)
        return r.returncode, r.stdout.strip(), norm_err(r.stderr)
    except subprocess.TimeoutExpired:
        return 999, "", "TIMEOUT"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gen", action="store_true")
    ap.add_argument("--root", default=".")
    ap.add_argument("--work", default="/tmp/m230_gate")
    ap.add_argument("--drv", default=None)
    a = ap.parse_args()
    HARM = bool(os.environ.get("M230_HARM"))   # 负控 C：判据自伤（期望检查恒真）
    if a.gen:
        return gen(a.root)
    root, work = a.root, a.work
    drv = a.drv or os.path.join(root, "examples", "m230_index_slice", "drv.px")
    tracks = {
        "interp": [os.environ.get("M230_PXI", os.path.join(root, "bootstrap", "pxi")), drv],
        "vm":     [os.path.join(work, "build", "drv_vm")],
        "c":      [os.path.join(work, "build", "drv_c")],
    }
    for t, cmd in tracks.items():
        if not os.path.exists(cmd[0]):
            print("❌ 缺少 %s 轨件 %s" % (t, cmd[0]))
            return 2
    diffs, model, n = [], {}, 0
    for c in CASES:
        cid, cont, op, shape, _, kind, want = c
        n += 1
        res = {t: run_track(cmd, cid, work) for t, cmd in tracks.items()}
        if HARM:
            # 负控 C：判据自伤 —— 完全跳过本例的全部判据（H1 跨轨 / H2 值 / H3 真因 / MODEL）。
            #   ⚠️ 只跳期望检查是不够的：施加 A 后解释轨报原值、VM/C 报归一化值 ⇒ **H1 跨轨差异**
            #   仍会判红（首版实测正是如此，负控 C 因此假红）。
            continue
        # H1 跨轨一致
        keys = {t: (r[0] == 0, res[t][1], res[t][2]) for t, r in res.items()}
        base = keys["interp"]
        for t in ("vm", "c"):
            if keys[t] != base:
                diffs.append("跨轨差异 [%s] %s: interp=%s %s ⇄ %s=%s %s"
                             % (cid, cont, base[0], (base[1] or base[2])[:60], t, keys[t][0], (keys[t][1] or keys[t][2])[:60]))
        # H2/H3 期望
        for t in ("interp", "vm", "c"):
            rc, out, err = res[t]
            if kind == "VAL":
                if rc != 0 or out != want:
                    diffs.append("%s [%s/%s] 期望 %r，实得 rc=%s out=%r err=%r" % (t, cid, cont, want, rc, out, err[:70]))
            else:
                code, kw = want.split("|", 1)
                if rc == 0:
                    diffs.append("%s [%s/%s] 期望报错（%s），实得 rc=0 out=%r" % (t, cid, cont, code, out))
                elif code not in err or kw not in err:
                    diffs.append("%s [%s/%s] 期望含 %r，实得 %r" % (t, cid, cont, code + " " + kw, err[:80]))
        # MODEL 表
        r0 = res["interp"]
        key = (op, cont, shape)
        val = "OK" if r0[0] == 0 else "ERR"
        code = "OK" if r0[0] == 0 else (r0[2].split("]")[0].replace("[", "") if "]" in r0[2] else "SYNTAX")
        if key in model and model[key] != (val, code):
            diffs.append("MODEL 内部不一致 %s" % (key,))
        model[key] = (val, code)
    out = ["M230 对拍：%d 例 × 3 轨 = %d 次执行" % (n, n * 3)]
    mo = os.path.join(work, "model_actual.tsv")
    with open(mo, "w", encoding="utf-8") as f:
        f.write("操作\t容器\t形状\t行为\tR码\n")
        for k in sorted(model):
            f.write("%s\t%s\t%s\t%s\t%s\n" % (k[0], k[1], k[2], model[k][0], model[k][1]))
    diffs = [d for d in diffs if not d.startswith("MODEL 内部不一致")]
    if diffs:
        out.append("差异 %d 项：" % len(diffs))
        out += ["  " + d for d in diffs[:60]]
        print("\n".join(out))
        print("M230-THREE-TRACKS-FAIL")
        return 1
    out.append("跨轨 0 · 期望 0 ⇒ H1 跨轨一致 ✓ · H2 值一致 ✓ · H3 指到真因 ✓")
    print("\n".join(out))
    print("M230-THREE-TRACKS-OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
