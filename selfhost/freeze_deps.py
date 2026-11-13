#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ═══════════════════════════════════════════════════════════════════════
# freeze_deps.py —— 发射冻结门的**传递依赖指纹**（M398 · 缺陷 761）
# ---------------------------------------------------------------
# 缺陷 761 是什么：emitc_freeze.sh 的 src 指纹只对**入口文件**做 sha256。
#   入口 import 的模块（selfhost/value.px、stdlib/strings.px …）改了，
#   入口 src 不变而产物变 ⇒ 门判【类别 B · 发射行为回归 · 必须查】。
#   实测影响面：524 个入口里 50 个有传递依赖；stdlib/strings.px 被 18 个入口依赖。
#   两天内造成两次误判（M397 前 --freeze 漏下游 / M397 r1 假红）。
#
# 本脚本只做两件事：
#   --mode sig     <files...>  → 每行 `<file>\tsrc=<16>\tdep=<16>`
#                    src = 入口自身 sha（诊断用，判据不看它）
#                    dep = 入口 + 递归 import 闭包的合并指纹（类别 A/B 的分界就靠它）
#   --mode explain <file>      → 人读：闭包清单 + 各自 sha16 + git 最近一次提交
#
# 解析规则**照抄编译器**（selfhost/cg_module.px:cg_find_module_path / cg_load_module）：
#   ① import "c/xxx"            → C 库绑定，不加载 .px（M42）
#   ② import "path/to/x.px"     → 相对**当前模块所在目录**（递归时换成模块目录）
#   ③ import std.x.y            → <repo>/stdlib/x/y.px 或 <repo>/stdlib/x/y/mod.px
#   ④ import x.y                → base_dir[/.px_modules[/子目录]]/x/y.px 或 x/y/mod.px
# 保守原则：**宁可多算不可少算** —— 多算只会多一点重定基，少算就是缺陷 761 本身。
#   因此解析失败的 import 把**字面量**也拼进指纹（改名/改写 import 同样触发 A 类）。
# ═══════════════════════════════════════════════════════════════════════
import os
import re
import sys
import hashlib
import subprocess

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
STDLIB = os.path.join(REPO, "stdlib")

IMP_RE = re.compile(r'^\s*import\s+(.+?)\s*$')
FROM_RE = re.compile(r'^\s*from\s+(\S+)\s+import\s+')


def sha16(path):
    h = hashlib.sha256()
    try:
        with open(path, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
    except OSError:
        return "READERR"
    return h.hexdigest()[:16]


def find_module(module, base_dir):
    """返回 (路径 或 None, 原因)。规则照 cg_find_module_path。"""
    if not module:
        return None, "empty"
    # ① C 库绑定
    if len(module) == 1 and module[0].startswith("c/") and ".px" not in module[0]:
        return None, "clib"
    # ② 相对路径
    if len(module) == 1 and ("/" in module[0] or ".px" in module[0]):
        rel = module[0]
        p = rel if rel.startswith("/") else os.path.join(base_dir, rel)
        p = os.path.normpath(p)
        if os.path.exists(p):
            return p, "rel"
        return None, "rel-missing"
    # ③ std.*
    if module[0] == "std":
        if len(module) < 2:
            return None, "std-bare"
        base = os.path.join(STDLIB, *module[1:])
        for cand in (base + ".px", os.path.join(base, "mod.px")):
            if os.path.exists(cand):
                return os.path.normpath(cand), "std"
        return None, "std-missing"
    # ④ 用户点分包
    bases = [base_dir]
    mods = os.path.join(base_dir, ".px_modules")
    if os.path.isdir(mods):
        bases.append(mods)
        try:
            for sub in sorted(os.listdir(mods)):
                full = os.path.join(mods, sub)
                if os.path.isdir(full) and "." not in sub:
                    bases.append(full)
        except OSError:
            pass
    for b in bases:
        p = os.path.join(b, *module)
        for cand in (p + ".px", os.path.join(p, "mod.px")):
            if os.path.exists(cand):
                return os.path.normpath(cand), "dotted"
        if len(module) == 1:
            single = os.path.join(b, module[0] + ".px")
            if os.path.exists(single):
                return os.path.normpath(single), "dotted-single"
    return None, "dotted-missing"


def imports_of(path):
    out = []
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                if line.lstrip().startswith("#"):
                    continue
                m = IMP_RE.match(line)
                if m:
                    body = m.group(1).split("#")[0].strip()
                    if not body:
                        continue
                    if body.startswith('"'):
                        end = body.find('"', 1)
                        if end > 0:
                            out.append([body[1:end]])
                            continue
                    out.append(body.split("."))
                    continue
                m = FROM_RE.match(line)
                if m:
                    out.append(m.group(1).split("."))
    except OSError:
        pass
    return out


def closure(entry):
    """返回 (闭包文件表（按路径排序，不含 entry）, 未解析字面量表)"""
    seen = {entry: True}
    deps = []
    unresolved = []
    queue = [entry]
    while queue:
        f = queue.pop(0)
        for mod in imports_of(f):
            found, why = find_module(mod, os.path.dirname(f))
            if found:
                if found not in seen:
                    seen[found] = True
                    deps.append(found)
                    queue.append(found)
            elif why != "clib":
                unresolved.append(".".join(mod))
    return sorted(deps), sorted(set(unresolved))


def dep_sig(entry):
    """dep = 入口 sha + 依赖闭包 sha + 未解析 import 字面量 的合并指纹。

    ⚠️ 入口自身**必须**计入：否则「无 import 的入口」dep 恒为常量，
    源码一改就会被误判成【类别 B · 源码未变而产物变】。
    """
    deps, unresolved = closure(entry)
    h = hashlib.sha256()
    h.update(("E %s %s\n" % (os.path.relpath(entry, REPO), sha16(entry))).encode())
    for p in deps:
        h.update(("D %s %s\n" % (os.path.relpath(p, REPO), sha16(p))).encode())
    for u in unresolved:
        h.update(("U %s\n" % u).encode())
    return h.hexdigest()[:16], deps, unresolved


def mode_sig(files):
    for rel in files:
        p = os.path.join(REPO, rel)
        sig, _, _ = dep_sig(p)
        print("%s\tsrc=%s\tdep=%s" % (rel, sha16(p), sig))


def git_last(p):
    try:
        out = subprocess.check_output(
            ["git", "log", "-1", "--format=%h %ad %s", "--date=short", "--", p],
            cwd=REPO, stderr=subprocess.DEVNULL, timeout=15)
        s = out.decode("utf-8", "replace").strip()
        return s if s else "(未提交/无历史)"
    except Exception:
        return "(git 不可用)"


def mode_explain(rel):
    p = os.path.join(REPO, rel)
    sig, deps, unresolved = dep_sig(p)
    print("入口  %s  src=%s  dep=%s" % (rel, sha16(p), sig))
    print("闭包  %d 个依赖文件（改其中任何一个都会让 dep 变 ⇒ 判【类别 A】而不是回归）" % len(deps))
    for d in deps:
        print("  %s  %-58s %s" % (sha16(d), os.path.relpath(d, REPO), git_last(d)))
    if unresolved:
        print("未解析 import %d 条（字面量已计入指纹）：" % len(unresolved))
        for u in unresolved:
            print("  %s" % u)


def main(argv):
    if len(argv) < 3:
        print("用法: freeze_deps.py --mode sig <files...> | --mode explain <file>", file=sys.stderr)
        return 2
    mode = argv[1]
    if mode == "--mode":
        mode = argv[2]
        args = argv[3:]
    else:
        print("未知模式 %s" % mode, file=sys.stderr)
        return 2
    if mode == "sig":
        mode_sig(args)
        return 0
    if mode == "explain":
        for a in args:
            mode_explain(a)
        return 0
    print("未知模式 %s" % mode, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv))
