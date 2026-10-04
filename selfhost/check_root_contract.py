#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# selfhost/check_root_contract.py —— GC 根登记栈「契约」守卫（M258 建立）
# ------------------------------------------------------------
# 为什么需要它：缺陷 460（`px_root_restore` 立即收缩 ⇒ 返回值在「交回调用方」的
#   窗口里失去根面）是**静态可判**的，却活了 30+ 个里程碑 —— 因为根栈的三条契约
#   此前只写在注释里，没有任何机器在看。本守卫把三条契约机械化：
#
#   J1 **收缩必须走延迟语义**（M183 缺陷 197 / M258 缺陷 460 的统一形态）：
#      任何「把 `g_px_roots_n` 改小」的函数必须同时写 `g_px_trunc_pending`
#      （= 把**物理回收**推迟到调用方「接住」那一刻）。
#      唯一豁免 = `px_root_restore_iso`（隔离点 longjmp 落点：被跳过的登记**必须立刻**
#      作废，否则成野根 —— 语义不同，理由写在 runtime.c 该函数注释里）。
#   J2 **作用域 API 成对**：同一函数内 `px_root_depth(` 与 `px_root_restore(`
#      **两边都必须有**（多出口 ⇒ 多个 restore 是**合法**的，判据只看「有没有」，
#      不看个数相等）；`px_root_iso_mark(` ⇄ `px_root_restore_iso(` 同。
#   J3 **无孤弹**：任何函数里出现 `px_root_pop(`，同函数内必须有 `px_root_push*` ——
#      「有 pop 无 push」必然弹到**外层作用域**的 marks 条目 ⇒ trunc_pending 被设成
#      过小值 ⇒ 收缩掉**仍然活跃**的帧的根（缺陷 459 同类形态）。
#   J4 **规模锚点**：扫描面（文件/函数/调用点）低于下限即判红 —— 防「扫描器静默失效
#      后恒绿」（本仓纪律：判据「找不到」先怀疑判据）。
#   ⚠️ 四个判据都**必须跳过 API 自身的定义体**（否则「函数里出现自己的名字」被误判）。
#
# 用法：check_root_contract.py [--root DIR] [--self-test] [--json]
# 退出码：0 通过 · 1 契约违例 · 2 用法错 · 3 扫描面异常（锚点不足）
# ============================================================
import argparse
import glob
import json
import os
import re
import sys

J1_IMMEDIATE_OK = {
    "px_root_restore_iso": "隔离点落点：longjmp 跳过的登记必须**立刻**作废（不设延迟，否则成野根）",
}
ANCHORS = {
    "min_files": 8,
    "min_funcs": 900,
    "min_root_api_calls": 20,
    "max_j1_exempt": 2,
}
API_NAMES = {"px_root_depth", "px_root_restore", "px_root_iso_mark", "px_root_restore_iso",
             "px_root_pop", "px_root_push", "px_root_push_keep", "px_root_keep"}


def strip_code(s):
    out = []
    i, n = 0, len(s)
    while i < n:
        c = s[i]
        if c == "/" and i + 1 < n and s[i + 1] == "*":
            j = s.find("*/", i + 2)
            if j < 0:
                i = n
                continue
            out.append("\n" * s.count("\n", i, j))
            i = j + 2
            continue
        if c == "/" and i + 1 < n and s[i + 1] == "/":
            j = s.find("\n", i)
            i = n if j < 0 else j
            continue
        if c in "'\"":
            q = c
            j = i + 1
            while j < n:
                if s[j] == "\\":
                    j += 2
                    continue
                if s[j] == q:
                    break
                j += 1
            if j >= n:
                i = n
                continue
            out.append("\n" * s.count("\n", i, j))
            i = j + 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


_RET = (r"(?:void|int|bool|char|LXValue|long|unsigned|const|double|float|size_t|"
        r"uint64_t|int64_t|ssize_t|uint32_t|int32_t|[A-Za-z_][A-Za-z0-9_]*[\* \t]+)")
FUNC_DEF = re.compile(
    r"^[ \t]*(?:static[ \t]+)?(?:inline[ \t]+)?" + _RET +
    r"([A-Za-z_][A-Za-z0-9_]*)[ \t]*\([^;{]*\)[ \t]*\{[ \t]*$"
)
FUNC_DEF_ANY = re.compile(
    r"^[ \t]*(?:static[ \t]+)?(?:inline[ \t]+)?[^;{=]*?([A-Za-z_][A-Za-z0-9_]*)[ \t]*\([^;{]*\)[ \t]*\{"
)


def functions(text):
    """行驱动切分（C）。返回 [(name, l0, body)]。`depth<=0` 收尾 —— 对 `.c` 足够。"""
    res = []
    lines = text.split("\n")
    depth, cur = 0, None
    for ln, line in enumerate(lines, 1):
        if depth == 0 and cur is None and "{" in line:
            m = FUNC_DEF.match(line) or FUNC_DEF_ANY.match(line)
            if m:
                cur = (m.group(1), ln, [])
        if cur is not None:
            cur[2].append(line)
        depth += line.count("{") - line.count("}")
        if cur is not None and depth <= 0:
            res.append((cur[0], cur[1], "\n".join(cur[2])))
            cur, depth = None, 0
    return res


def _cnt(body, name, self_name=None):
    """计数；`self_name == name` 时扣掉**定义行自身**那一次出现。"""
    c = len(re.findall(r"\b%s\s*\(" % name, body))
    if self_name == name:
        c -= 1
    return c if c > 0 else 0


def check(root):
    files = sorted(glob.glob(os.path.join(root, "runtime", "*.c")))
    bad, anchor_bad = [], []
    nfuncs, calls = 0, 0

    for f in files:
        base = os.path.basename(f)
        try:
            text = strip_code(open(f, encoding="utf-8", errors="replace").read())
        except OSError:
            continue
        for fname, l0, body in functions(text):
            nfuncs += 1
            _ = fname in API_NAMES   # 仅作可读性提示：判据一律按「扣掉自引用」处理

            # ---- J1：收缩必须走延迟语义（**对 API 自身的定义同样成立** —— 缺陷 460 就在
            #       `px_root_restore` 的定义体里；此处**不**跳过 API 定义）----
            if re.search(r"\bg_px_roots_n\s*=", body):
                if fname not in J1_IMMEDIATE_OK and not re.search(r"\bg_px_trunc_pending\s*=", body):
                    bad.append(("J1", base, l0,
                                "函数 %s 改小 g_px_roots_n 但未写 g_px_trunc_pending"
                                "（必须走延迟收缩；若确属立即收缩，请加进 J1 豁免表并写明理由）" % fname))

            # ---- J2：作用域 API 成对（只看「有没有」，多出口多 restore 合法）----
            if True:
                for a, b in (("px_root_depth", "px_root_restore"),
                             ("px_root_iso_mark", "px_root_restore_iso")):
                    na, nb = _cnt(body, a, fname), _cnt(body, b, fname)
                    calls += na + nb
                    if na and not nb:
                        bad.append(("J2", base, l0, "函数 %s 调用 %s 但无 %s（作用域不归还）" % (fname, a, b)))
                    if nb and not na:
                        bad.append(("J2", base, l0, "函数 %s 调用 %s 但无 %s（归还了没记的深度）" % (fname, b, a)))
                    if na and nb and nb < na:
                        bad.append(("J2", base, l0,
                                    "函数 %s 内 %s×%d > %s×%d（归还点少于登记点 ⇒ 必有出口漏归还）"
                                    % (fname, a, na, b, nb)))

            # ---- J3：无孤弹 ----
            if True:
                npop = _cnt(body, "px_root_pop", fname)
                npush = _cnt(body, "px_root_push", fname) + _cnt(body, "px_root_push_keep", fname)
                if npop and not npush:
                    bad.append(("J3", base, l0,
                                "函数 %s 有 px_root_pop 但整函数无 px_root_push* ⇒ 必然弹到外层作用域条目" % fname))

    if len(files) < ANCHORS["min_files"]:
        anchor_bad.append("扫描文件数 %d < %d" % (len(files), ANCHORS["min_files"]))
    if nfuncs < ANCHORS["min_funcs"]:
        anchor_bad.append("解析函数数 %d < %d（解析器可能失效）" % (nfuncs, ANCHORS["min_funcs"]))
    if calls < ANCHORS["min_root_api_calls"]:
        anchor_bad.append("作用域 API 调用点 %d < %d（扫描面可能变窄）" % (calls, ANCHORS["min_root_api_calls"]))
    if len(J1_IMMEDIATE_OK) > ANCHORS["max_j1_exempt"]:
        anchor_bad.append("J1 豁免表 %d 条 > 上限 %d（豁免不该增长）" % (len(J1_IMMEDIATE_OK), ANCHORS["max_j1_exempt"]))
    return {"files": len(files), "funcs": nfuncs, "calls": calls,
            "bad": bad, "anchor_bad": anchor_bad}


def report(res, verbose=True):
    if verbose:
        print("扫描：文件 %d · 函数 %d · 作用域 API 调用点 %d"
              % (res["files"], res["funcs"], res["calls"]))
        print("J1 豁免：%s" % ", ".join(sorted(J1_IMMEDIATE_OK)))
        for j, f, ln, msg in res["bad"]:
            print("❌ [%s] %s:%d  %s" % (j, f, ln, msg))
        for a in res["anchor_bad"]:
            print("❌ [J4] %s" % a)
        if not res["bad"] and not res["anchor_bad"]:
            print("✅ ROOT-CONTRACT-OK（J1 延迟收缩 · J2 成对 · J3 无孤弹 · J4 锚点）")
    return (1 if res["bad"] else 0) or (3 if res["anchor_bad"] else 0)


def self_test(tmp):
    os.makedirs(os.path.join(tmp, "runtime"), exist_ok=True)
    p = os.path.join(tmp, "runtime", "runtime.c")
    good = '''
static void px_root_keep(const LXValue* v) {
    if (g_px_trunc_pending >= 0) { if (t < g_px_roots_n) g_px_roots_n = t; g_px_trunc_pending = -1; }
}
void px_root_depth(int* m) { if (m) *m = 0; }
void px_root_restore(int rd, int rm) {
    if (g_px_roots_n > rd) g_px_trunc_pending = rd;
}
void px_root_iso_mark(void) { t_iso_roots = g_px_roots_n; }
void px_root_restore_iso(void) {
    if (t_iso_roots >= 0 && g_px_roots_n > t_iso_roots) g_px_roots_n = t_iso_roots;
    g_px_trunc_pending = -1;
}
void px_root_pop(void) { (void)0; }
static int qp_dec_fields(void) {
    int rm = 0; int rd = px_root_depth(&rm);
    px_root_push(); px_root_pop();
    if (0) { px_root_restore(rd, rm); return 0; }
    px_root_restore(rd, rm);
    return 0;
}
static int iso_one(void) { px_root_iso_mark(); px_root_restore_iso(); return 0; }
'''
    def run(text):
        open(p, "w").write(text)
        return report(check(tmp), verbose=False)

    saved = dict(ANCHORS)
    ANCHORS.update({"min_files": 1, "min_funcs": 5, "min_root_api_calls": 1})
    try:
        open(p, "w").write(good)
        ok = run(good) == 0
        a = run(good.replace("g_px_trunc_pending = rd;", "g_px_roots_n = rd;")) == 1
        b = run(good.replace("px_root_restore(rd, rm);", "(void)0;")) == 1
        c = run(good + "\nstatic void stray(void) { px_root_pop(); }\n") == 1
        ANCHORS["min_funcs"] = 10 ** 6
        d = run(good) == 3
        ANCHORS["min_funcs"] = saved["min_funcs"]
    finally:
        ANCHORS.clear()
        ANCHORS.update(saved)
    print("自证：好样本 %s · NC-A(J1) %s · NC-B(J2) %s · NC-C(J3) %s · NC-D(J4) %s"
          % ("✅" if ok else "❌", a, b, c, d))
    return 0 if (ok and a and b and c and d) else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--tmp", default="/tmp/m258_rootctl_st")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()
    if a.self_test:
        rc = self_test(a.tmp)
        print("SELF-TEST-%s" % ("OK" if rc == 0 else "FAIL"))
        return rc
    res = check(a.root)
    if a.json:
        print(json.dumps(res, ensure_ascii=False))
        return (1 if res["bad"] else 0) or (3 if res["anchor_bad"] else 0)
    rc = report(res)
    print("ROOT-CONTRACT-%s" % ("OK" if rc == 0 else ("EXEMPT" if rc == 3 else "VIOLATION")))
    return rc


if __name__ == "__main__":
    sys.exit(main())
