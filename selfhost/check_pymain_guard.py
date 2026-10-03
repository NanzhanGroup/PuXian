#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
selfhost/check_pymain_guard.py —— **被 import 的模块必须有 `__main__` 守卫**（M247 · 缺陷 418 机械化）
================================================================================================
为什么必须有它（M246 实锤）：
    `examples/m246_compound_assign/pair_check.py` 写了
        from three_tracks import norm_err, run
    而那份 `three_tracks.py` 末尾是**模块级** `sys.exit(main())`
    ⇒ **import 动作本身执行了整个对拍并 `sys.exit`** ⇒ 新脚本的后续代码根本没跑。
    **症状极具误导性**：日志里出现的是**被 import 方**的输出，
    排查者会以为「新脚本没写对」，而不是「import 了一份独立 runner」。

    ⭐ 本仓是**系列复用模式**（`three_tracks.py` 同名 12 份、`gen_probes.py` 5 份、
    `negctl.py` 7 份 …），「新门复用同族 runner」正是踩这个坑的标准动作 ⇒ 靠人记必然复发。

判据（唯一 · 三条同时成立才判红）
--------------------------------
    ① 文件 F **被某个 .py 指向**（`from <模块名> import …` / `import <模块名>` 解析到 F）；
    ② F 在**顶层**（不在任何函数 / 不在 `if __name__` 块内）调用 `sys.exit(`；
    ③ F **不含** `if __name__ == "__main__":` 守卫。
    ⇒ 判红并**指名**文件与那一行（= 「import 它会执行并终止调用方进程」）。

⚠️ 解析一律走 `ast`（**不许用正则**）
------------------------------------
    首版用 `^\\s*from\\s+(\\w+)\\s+import` ⇒ **本脚本自己的文档字符串**里那句示例代码
    被当成了真 import ⇒ 它又不在 `three_tracks` 的同目录 ⇒ 按歧义规则**指向全部 12 份**
    ⇒ 误报 11 条。**「判据被自己的说明文字骗了」**（M219 同族：判据要贴着**契约**写）。
    ⇒ 用 `ast.parse` 取 `Import`/`ImportFrom` 节点：注释与字符串**天然不可见**。

import 目标怎么解析（**难点，必须精确**）
----------------------------------------
    同名多份不能一视同仁 —— 否则会**误伤**其余 11 份
    （它们在**自己的**门里是独立 runner，那是正确形态）。规则：

      · 引用方与候选**同目录** ⇒ 指向同目录那份（= Python `sys.path[0]` 语义，本仓实际用法）；
      · 否则若该模块名**全局唯一** ⇒ 指向它；
      · 否则（多份且都不在同目录）⇒ **歧义 ⇒ 全部计入**（保守：宁可多报也不漏报）。

为什么不是「被 import 就必须有守卫」（更宽的口径）
--------------------------------------------------
    纯库文件（`stubs.py` 之类）被 import 是**正常形态**，没有 `__main__` 也是对的
    ⇒ 宽口径会**瞎报**（M213 缺陷 300 / M219 教训）。本判据只咬
    「**import 了一个独立 runner**」这一种真实危害 —— 且**限定在"真的被指向"**。

ℹ️ 潜伏面（不判红，只报告）
--------------------------
    顶层有 `sys.exit(`、无守卫、但**当前没被任何文件指向**的 runner ⇒ 列出（含行号）。
    **它们是「将来复用前要先加守卫」的清单** —— 本仓历史证明这类文件迟早被复制进新门。
    当前实况见 `docs/NEW_GATE_CHECKLIST.md` §二（口径已登记）。

用法
----
    python3 selfhost/check_pymain_guard.py              # 扫描（仓库根由脚本位置推出）
    python3 selfhost/check_pymain_guard.py --self-test  # 自证（7 判据含 1 负控），失败 rc=2
    python3 selfhost/check_pymain_guard.py --quiet      # 只输出结论行

退出码：0 无违例 / 1 有违例 / 2 自证失败
"""
import ast
import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

SCAN_DIRS = ("examples", "selfhost", "tools", "packaging", "docs", "stdlib")
SKIP_DIRS = {"build", "__pycache__", ".rtcache", ".git", "third_party"}


# ---------------------------------------------------------------- 解析（ast）
def read(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return f.read()
    except OSError:
        return ""


def parse(path):
    try:
        return ast.parse(read(path))
    except SyntaxError:
        return None


def imports_of(tree):
    """→ [顶层模块名…]（含函数内的 import —— 它们在**运行时**也会执行）"""
    out = []
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom):
            if node.level == 0 and node.module:
                out.append(node.module.split(".")[0])
        elif isinstance(node, ast.Import):
            for al in node.names:
                out.append(al.name.split(".")[0])
    return out


def _is_sys_exit(call):
    f = getattr(call, "func", None)
    return (isinstance(f, ast.Attribute) and f.attr == "exit"
            and isinstance(f.value, ast.Name) and f.value.id == "sys")


def top_sysexit_lineno(tree):
    """**顶层**（`tree.body` 直接层）的 `sys.exit(...)` 行号；无则 None

    ⚠️ 只看 `tree.body` 直接层 ⇒ `if __name__ == "__main__": sys.exit(...)` 与
    函数体内的 `sys.exit(...)` **都不算**（前者是已被守卫包住的正确形态）。
    """
    for node in tree.body:
        if isinstance(node, ast.Expr) and isinstance(node.value, ast.Call) \
                and _is_sys_exit(node.value):
            return node.lineno
    return None


def has_main_guard(tree):
    """顶层是否存在 `if __name__ == "__main__":`"""
    for node in tree.body:
        if not isinstance(node, ast.If):
            continue
        t = node.test
        if not isinstance(t, ast.Compare) or not isinstance(t.left, ast.Name) \
                or t.left.id != "__name__":
            continue
        for c in t.comparators:
            if isinstance(c, ast.Constant) and c.value == "__main__":
                return True
    return False


# ---------------------------------------------------------------- 扫描
def collect(root):
    """→ {模块名: [路径…]}，只收 SCAN_DIRS 下的 .py"""
    byname = {}
    for base in SCAN_DIRS:
        d = os.path.join(root, base)
        if not os.path.isdir(d):
            continue
        for dp, dns, fns in os.walk(d):
            dns[:] = [x for x in dns if x not in SKIP_DIRS]
            for fn in fns:
                if fn.endswith(".py"):
                    byname.setdefault(fn[:-3], []).append(os.path.join(dp, fn))
    return byname


def resolve(importer, mod, byname):
    """`from <mod> import …` 在 importer 里指向哪些文件（同目录优先 → 全局唯一 → 歧义全算）"""
    cands = byname.get(mod, [])
    if not cands:
        return []
    same = [p for p in cands if os.path.dirname(p) == os.path.dirname(importer)]
    return same if same else cands


def scan(root):
    """→ (red[], latent[], stats)；元素为 (绝对路径, 行号)

    red     = 命中判据（**被指向** + 顶层 sys.exit + 无守卫）
    latent  = 顶层 sys.exit + 无守卫，但当前没被指向（ℹ️ 潜伏面）
    """
    byname = collect(root)
    trees = {p: parse(p) for paths in byname.values() for p in paths}

    pointed = set()
    n_unparsed = 0
    for p, tree in trees.items():
        if tree is None:
            n_unparsed += 1
            continue
        for mod in imports_of(tree):
            if mod in byname:
                pointed.update(resolve(p, mod, byname))

    red, latent, nfiles, nguarded = [], [], 0, 0
    for _name, paths in sorted(byname.items()):
        for p in sorted(paths):
            nfiles += 1
            tree = trees.get(p)
            if tree is None:
                continue
            if has_main_guard(tree):
                nguarded += 1
                continue
            ln = top_sysexit_lineno(tree)
            if ln is None:
                continue
            (red if p in pointed else latent).append((p, ln))

    stats = {"files": nfiles, "guarded": nguarded, "pointed": len(pointed),
             "names": len(byname), "unparsed": n_unparsed}
    return red, latent, stats


def report(red, latent, stats, root=ROOT, quiet=False):
    if not quiet:
        print("扫描 %d 个 .py（%d 个模块名）· 其中 %d 个含 __main__ 守卫"
              % (stats["files"], stats["names"], stats["guarded"]))
        print("被 import 指向的本地文件：%d 个" % stats["pointed"])
        if stats["unparsed"]:
            print("⚠️ ast 解析失败（未纳入判据）：%d 个" % stats["unparsed"])
        if latent:
            print()
            print("ℹ️ 潜伏面（顶层有 sys.exit 且无守卫，当前**未被**指向）：%d 个" % len(latent))
            for p, ln in latent:
                print("     %s:%d" % (os.path.relpath(p, root), ln))
            print("     ⇒ 复用其中任何一份之前，**先给它加 `if __name__ == \"__main__\":` 守卫**")
            print("       （本仓「新门复用同族 runner」是系列复用模式，见 docs/NEW_GATE_CHECKLIST.md §二）")
    print()
    if red:
        print("❌ 命中判据（被 import 指向 + 顶层 sys.exit + 无守卫）：%d 个" % len(red))
        for p, ln in red:
            print("     %s:%d  ← import 它会执行并终止调用方进程" % (os.path.relpath(p, root), ln))
        print("     ⇒ 把顶层主程序包进 `if __name__ == \"__main__\":`（被 import 的符号留模块级）")
        print("PYMAIN-GUARD-FAIL 违例 %d 个" % len(red))
        return 1
    tail = "（被指向的文件中，含顶层 sys.exit 的都已具备守卫）" if stats["pointed"] else ""
    print("✅ PYMAIN-GUARD-OK 违例 0 个%s" % tail)
    return 0


# ---------------------------------------------------------------- 自证
SELFTEST_FILES = {
    # 判红：被同目录的 user.py 指向 + 顶层 sys.exit + 无守卫
    "mod_red.py": 'import sys\n\n\ndef work():\n    return 1\n\nprint("side effect")\nsys.exit(work())\n',
    # 绿：被指向 + 顶层 sys.exit + **有**守卫
    "mod_guarded.py": ('import sys\n\n\ndef work():\n    return 1\n\n\n'
                       'if __name__ == "__main__":\n    sys.exit(work())\n'),
    # 绿：纯库（被指向，但无顶层 sys.exit）
    "mod_lib.py": 'def helper():\n    return 2\n',
    # ℹ️ 潜伏面：顶层 sys.exit + 无守卫，但**没有**谁指向它
    "mod_latent.py": 'import sys\n\n\ndef work():\n    return 3\n\nsys.exit(0 if work() else 1)\n',
    # 绿：顶层调了 main()，但 sys.exit 在函数里（不在 tree.body 直接层）
    "mod_indent.py": 'import sys\n\n\ndef main():\n    sys.exit(0)\n\n\nmain()\n',
    # ⭐ 绿：只在**文档字符串**里出现 `from mod_red import …` —— 正则会被骗，ast 不会
    "mod_docstring.py": ('"""说明：本文件被这样引用 —— from mod_red import work\n"""\n'
                         'import sys\n\n\nif __name__ == "__main__":\n    sys.exit(0)\n'),
    # 引用方
    "user.py": ('import sys\nfrom mod_red import work\n\n\n'
                'if __name__ == "__main__":\n    sys.exit(work())\n'),
}
# 同名但**不在引用方同目录** ⇒ 不该被判红（只进潜伏面）—— 自证 ⑦
SELFTEST_SUB = {"sub/mod_red.py": 'import sys\n\nprint("other copy")\nsys.exit(9)\n'}


def _mkfixture():
    t = tempfile.mkdtemp(prefix="pymain-")
    d = os.path.join(t, "examples", "fake")
    os.makedirs(os.path.join(d, "sub"))
    for fn, body in list(SELFTEST_FILES.items()) + list(SELFTEST_SUB.items()):
        with open(os.path.join(d, fn), "w", encoding="utf-8") as f:
            f.write(body)
    return t


def selftest():
    t = _mkfixture()
    fails = []
    red, latent, _ = scan(t)
    redset = {os.path.relpath(p, t) for p, _ in red}
    latset = {os.path.relpath(p, t) for p, _ in latent}

    def any_in(needle, s):
        return any(x.endswith(needle) for x in s)

    # ① 判红面**精确相等**（不多不少 —— 过宽与过窄都要被抓住）
    if redset != {os.path.join("examples", "fake", "mod_red.py")}:
        fails.append("① 判红集应恰为 {examples/fake/mod_red.py}，实得 %s" % sorted(redset))
    # ② 有守卫的被指向模块不得判红
    if any_in("mod_guarded.py", redset):
        fails.append("② mod_guarded.py（有守卫）被误判红")
    # ③ 纯库被指向不得判红（也不该进潜伏面）
    if any_in("mod_lib.py", redset) or any_in("mod_lib.py", latset):
        fails.append("③ mod_lib.py（纯库）被误伤")
    # ④ 未被指向的 runner ⇒ 只进潜伏面，不判红
    if any_in("mod_latent.py", redset):
        fails.append("④ mod_latent.py（未被指向）被误判红 —— 判据过宽")
    if not any_in("mod_latent.py", latset):
        fails.append("④b mod_latent.py 应出现在潜伏面")
    # ⑤ 函数内的 sys.exit 不算顶层
    if any_in("mod_indent.py", redset):
        fails.append("⑤ mod_indent.py 的 sys.exit 在函数内，被误判红")
    # ⑥ 负控（判据自伤）：让「有守卫」恒为真 ⇒ 判红必须**消失**
    saved = globals()["has_main_guard"]
    try:
        globals()["has_main_guard"] = lambda _tree: True
        red2, _, _ = scan(t)
        if red2:
            fails.append("⑥ 负控：判据失去牙后仍报出违例（说明比对路径有误）")
    finally:
        globals()["has_main_guard"] = saved
    # ⑦ resolve 规则：同名不同目录 ⇒ 只认同目录那份，另一份进潜伏面
    if any_in(os.path.join("sub", "mod_red.py"), redset):
        fails.append("⑦ sub/mod_red.py（同名但非同目录）被误判红 —— resolve 规则失效")
    if not any_in(os.path.join("sub", "mod_red.py"), latset):
        fails.append("⑦b sub/mod_red.py 应出现在潜伏面")
    # ⑧ ⭐ 文档字符串里的 import 不算数（正则会被骗，ast 不会）
    if any_in("mod_docstring.py", redset):
        fails.append("⑧ mod_docstring.py 仅 docstring 里出现 import，被误判红")

    for ln in fails:
        print("SELFTEST-FAIL " + ln)
    if fails:
        print("PYMAIN-GUARD-SELFTEST-FAIL %d 项" % len(fails))
        return 2
    print("PYMAIN-GUARD-SELFTEST-OK 8 判据（①判红集精确相等 ②有守卫不报 ③纯库不误伤 "
          "④未被指向只入潜伏面 ⑤缩进/守卫内不算顶层 ⑥负控·判据自伤 "
          "⑦同名不同目录按 resolve 分流 ⑧docstring 里的 import 不算数）")
    return 0


def main():
    args = sys.argv[1:]
    for a in args:
        if a not in ("--self-test", "--quiet"):
            print("未知选项: %s" % a, file=sys.stderr)
            print("用法: check_pymain_guard.py [--self-test|--quiet]", file=sys.stderr)
            return 2
    if "--self-test" in args:
        return selftest()
    red, latent, stats = scan(ROOT)
    return report(red, latent, stats, quiet="--quiet" in args)


if __name__ == "__main__":
    sys.exit(main())
