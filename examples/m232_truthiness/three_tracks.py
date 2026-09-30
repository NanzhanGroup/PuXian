#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M232 门 · 三轨对拍器（「真值性表 + 短路族」）

判据
----
① **三轨输出逐字节一致**（本门的观察量已是「一行字符串」，直接比即可）；
② 与 `MODEL.tsv` **双向**核对：
     实测有、表里没有 ⇒ **漏登记**判红；表里有、实测没有 ⇒ **登记过期**判红；
③ **缺陷 347 的精确形状**独立判据（不依赖「三轨一致」）：
     · `bytes("")` 的真值性必须为 **FALSY**（与 `""` / `[]` / `{}` 同族）；
     · `()`       的真值性必须为 **FALSY**；
     · 由此派生的 4 条短路后果必须一致（`and`/`or` 对这两个值的**求值与否**）。
   ⇒ 为什么单独一层：负控 B 会让**编译轨也**漏同一个分支 ⇒ ① 依然绿（三轨一致地错），
     只有本层能独立判红。这正是本仓那条立论「**三轨一致 ≠ 正确**」的判据化。

诊断通道
--------
解释轨与编译轨的错误通道差异是**已登记缺陷 186**，不属本轮判据 ⇒ 归一化时抹掉前缀，
只留 `R码 + 词条`（若有）。归一化规则写在 norm_err() 里（不许「看着不一样就放过」）。
"""
import argparse, json, os, re, subprocess, sys


def norm(out):
    """→ ('VAL', 行) | ('ERR', R码, 词条) | ('OTHER', 原文)"""
    lines = [l for l in out.strip().split("\n") if l.strip() != ""]
    if not lines:
        return ("OTHER", "<空输出>")
    o = lines[0].strip()
    m = re.search(r'(R\d{4})', o)
    if m:
        rc = m.group(1)
        tail = o[m.start() + len(rc):].lstrip(":] \t")
        tail = re.sub(r'^\d+:\d+:\s*', '', tail).strip()
        return ("ERR", rc, tail)
    if "语法错误" in o or "E20" in o:
        return ("ERR", "SYNTAX", o[-60:])
    return ("VAL", o)


def run(cmd, work, idx, timeout=30):
    env = dict(os.environ)
    env["M232_CASE"] = str(idx)
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout,
                           cwd=work, env=env)
        return norm(r.stdout or r.stderr)
    except subprocess.TimeoutExpired:
        return ("OTHER", "TIMEOUT")


def load_cases(d):
    cases = []
    for ln in open(os.path.join(d, "cases.tsv"), encoding="utf-8"):
        f = ln.rstrip("\n").split("\t")
        if len(f) >= 6 and f[0] != "idx":
            cases.append((int(f[0]), f[1], f[2], f[3], f[4], f[5]))
    return cases


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--work", default="/tmp/m232_gate")
    ap.add_argument("--drv", default=None)
    ap.add_argument("--json", default=None)
    ap.add_argument("--shape347", action="store_true")
    # ⚠️ `--harm` = **判据自伤**（只给负控 C 用）：跳过 ② 逐轨 MODEL 核对。
    #   用途：证明「负控 B 的红」确实来自判据本身 ⇒ 关掉判据后红必须消失。
    #   正常路径**不得**带此开关（verify.sh 只在自己的负控 C 段使用）。
    ap.add_argument("--harm", action="store_true")
    a = ap.parse_args()
    root, work = os.path.abspath(a.root), a.work
    d = os.path.join(root, "examples", "m232_truthiness")
    # ⚠️ 必须**绝对化**：下面会用 cwd=<驱动器所在目录> 起进程（相对 import 才命中），
    #   相对路径会被二次拼接（M231 首版同坑）⇒ 解释轨读文件失败、三轨全假红。
    drv = os.path.abspath(a.drv or os.path.join(d, "drv.px"))
    cases = load_cases(d)
    workdir = os.path.dirname(drv)

    tracks = {
        "interp": [os.environ.get("M232_PXI", os.path.join(root, "bootstrap", "pxi")), drv],
        "vm":     [os.path.join(work, "build", "drv_vm")],
        "c":      [os.path.join(work, "build", "drv_c")],
    }

    # ---- ③ 缺陷 347 精确形状（独立于「三轨一致」）----
    def shape347():
        # 真值性真值（从三轨输出的第 2 列取）—— 期望全部 FALSY
        must_falsy = ["str_empty", "bytes_empty", "list_empty", "dict_empty", "tuple_empty", "null_v"]
        idx_of = {c[2]: c[0] for c in cases}
        bad = 0
        for tag in must_falsy:
            if tag not in idx_of:
                print("  ❌ [347] 用例缺失: %s" % tag); bad += 1; continue
            for t, cmd in tracks.items():
                res = run(cmd, workdir, idx_of[tag])
                if res[0] != "VAL":
                    print("  ❌ [347] %s/%s ⇒ %s（期望可运行）" % (tag, t, res)); bad += 1; continue
                cols = res[1].split("|")
                if len(cols) < 5 or cols[1] != "false":
                    print("  ❌ [347] %s/%s ⇒ bool=%s（期望 false）" % (tag, t, cols[1] if len(cols) > 1 else "?"))
                    bad += 1
        # 派生的 4 条短路后果：and/or 对这两个值的「求值与否」必须是「不求值」
        deriv = [("and_ebytes", "[]"), ("or_ebytes", "[A]"),
                 ("and_etuple", "[]"), ("or_etuple", "[A]")]
        # 这里期望与 MODEL 一致：两者皆为 FALSY ⇒ and 不求值、or 求值
        for tag, want_log in deriv:
            if tag not in idx_of:
                print("  ❌ [347] 用例缺失: %s" % tag); bad += 1; continue
            for t, cmd in tracks.items():
                res = run(cmd, workdir, idx_of[tag])
                if res[0] != "VAL":
                    print("  ❌ [347] %s/%s ⇒ %s" % (tag, t, res)); bad += 1; continue
                got_log = res[1].rsplit("|", 1)[-1]
                if got_log != want_log:
                    print("  ❌ [347] %s/%s ⇒ log=%s（期望 %s）" % (tag, t, got_log, want_log))
                    bad += 1
        print("  [347] 形状判据：%d 项检查 · %d 处不符" % ((len(must_falsy) + len(deriv)) * 3, bad))
        return bad

    if a.shape347:
        n = shape347()
        print("M232-SHAPE347-OK" if n == 0 else "M232-SHAPE347-FAIL")
        return 0 if n == 0 else 1

    for t, cmd in tracks.items():
        if not os.path.exists(cmd[0]):
            print("❌ 缺少 %s 轨件 %s" % (t, cmd[0]))
            return 2

    diffs, actual = [], {}
    for idx, kind, tag, expr, _exp, _b in cases:
        res = {t: run(cmd, workdir, idx) for t, cmd in tracks.items()}
        kinds = {res[t][0] for t in tracks}
        if len(kinds) != 1:
            diffs.append(("响亮性", tag, expr, res))
        else:
            k = kinds.pop()
            if k == "VAL":
                if len({res[t][1] for t in tracks}) != 1:
                    diffs.append(("值不同", tag, expr, res))
            else:
                if len({(res[t][1], res[t][2]) for t in tracks}) != 1:
                    diffs.append(("错误不同", tag, expr, res))
        actual[tag] = {t: (kind, res[t][1] if res[t][0] == "VAL"
                           else "%s:%s" % (res[t][1], res[t][2])) for t in tracks}

    # ② MODEL 双向 —— ⚠️ 对**每一条轨**核，不是只核参考轨
    #   理由：本仓立论「三轨一致 ≠ 正确」—— 若只核解释轨，则「编译两轨一致地错」
    #   且解释轨恰好对时不会被抓；反之亦然。逐轨核才能覆盖两种方向。
    model = {}
    for ln in open(os.path.join(d, "MODEL.tsv"), encoding="utf-8"):
        f = ln.rstrip("\n").split("\t")
        if len(f) >= 3 and f[0] != "tag":
            model[f[0]] = f[2]

    def observed(kind, line):
        """从实测行里取出「该 kind 应当与 MODEL 对齐的那一段」

        · T（真值性表）：行 = `tag|bool|not|T/F|and|or` ⇒ 取第 4 列（`if` 门的结果）；
        · S（短路族）：行 = `tag|值|日志`   ⇒ 取 `值|日志`；
        · P（管道）  ：行 = `tag|输出`      ⇒ 取 `输出`。
        为什么按 kind 分：三种面的「观察量形状」不同，用同一把尺子量会**假红**
        （首版就是把真值性整行拿去比 `T`/`F`，46 例里 30 例假红）。
        """
        if "|" not in line:
            return line
        cols = line.split("|")
        if kind == "T":
            return cols[3] if len(cols) > 3 else line
        return line.split("|", 1)[1]

    miss = sorted(set(model) - set(actual))
    extra = sorted(set(actual) - set(model))
    mismatch = []
    if a.harm:
        miss, extra = [], []
    for tag in ([] if a.harm else sorted(set(model) & set(actual))):
        got = {t: observed(actual[tag][t][0], actual[tag][t][1]) for t in ("interp", "vm", "c")}
        bad = [t for t in ("interp", "vm", "c") if got[t] != model[tag]]
        if bad:
            mismatch.append((tag, model[tag], got, bad))

    print("例数 %d · 三轨差异 %d · MODEL 不一致 %d · 漏登记 %d · 过期 %d"
          % (len(cases), len(diffs), len(mismatch), len(extra), len(miss)))
    for kind_, tag, expr, res in diffs[:20]:
        print("  ❌ [%s] %s  %s" % (kind_, tag, expr))
        for t in ("interp", "vm", "c"):
            print("       %-6s %s" % (t, res[t]))
    for tag, want, got, bad in mismatch[:20]:
        print("  ❌ [MODEL] %s：表 %s ⇄ 不符轨 %s" % (tag, want, ",".join(
            "%s=%s" % (t, got[t]) for t in bad)))
    for t in extra[:20]:
        print("  ❌ [MODEL] 漏登记 %s（实测 %s）" % (t, actual[t]["interp"][1]))
    for t in miss[:20]:
        print("  ❌ [MODEL] 登记过期 %s" % t)
    if a.json:
        json.dump({"n": len(cases), "diffs": len(diffs),
                   "model_bad": len(mismatch) + len(miss) + len(extra), "actual": actual},
                  open(a.json, "w"), ensure_ascii=False, indent=1)

    ok = (not diffs) and (not mismatch) and (not miss) and (not extra)
    print("M232-THREE-TRACKS-OK" if ok else "M232-THREE-TRACKS-FAIL")
    return 0 if ok else 1


sys.exit(main())
