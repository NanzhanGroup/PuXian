#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""S13 不变量检查器（**注释容忍** · 全仓 runtime/*.c）

不变量（M208 立 · M209 收口）：
  **「构造与登记之间不得存在安全点」** ⇒ 惯用法必须是**原子**的
  `px_root_push_keep(x);`，不得出现两段式
      px_root_push();
      PX_KEEP(x);
  （两次 gc_unblock_stop ⇒ 中间即窗口）。

为什么单独写一个检查器（M209 教训）：
  M208 门里的判据用的是**整行正则** `px_root_push\(\);[ \t]*\n[ \t]*PX_KEEP\(`，
  ⇒ **行尾带注释的形态（`px_root_push();   // …`）整族漏判**，
  且扫描面只覆盖 3 个文件 ⇒ 报「相邻两段式 = 0」是**假绿**（实测还有 30 处）。
  本检查器：① 先**剥注释**、② 把 `\\` 续行拼成逻辑行、③ 覆盖全部 `runtime/*.c`、
  ④ 两条规则（同一逻辑行内共现 / 相邻逻辑行两段式）。

用法：s13_check.py [--files ...] [--self-test]
退出码：0 = 无违例；1 = 有违例；2 = 自证失败。
"""
import glob
import re
import sys
import os

RULE_NAME = "S13"


def strip_comments(text):
    """去掉 // 行注释与 /* */ 块注释（保守：字符串内的 // 会被误剥，本仓可从
    "违例数上升" 观察到 —— 自证里用 fixture 覆盖两种形态）。"""
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    out = []
    for ln in text.split('\n'):
        i = ln.find('//')
        out.append(ln[:i] if i >= 0 else ln)
    return out


def join_continuations(lines):
    """把以 \\ 结尾的物理行拼成逻辑行，返回 [(行号, 内容)]"""
    res = []
    buf = ''
    start = 0
    for idx, ln in enumerate(lines, 1):
        s = ln.rstrip()
        if buf == '':
            start = idx
        if s.endswith('\\'):
            buf += s[:-1] + ' '
            continue
        res.append((start, buf + ln))
        buf = ''
    if buf:
        res.append((start, buf))
    return res


def violations(path):
    raw = open(path, encoding='utf-8').read()
    lines = strip_comments(raw)
    logical = join_continuations(lines)
    v = []
    stripped = [l.strip() for l in lines]

    # 规则 1：同一逻辑行里同时出现 `px_root_push()` 与 `PX_KEEP(`
    for ln, content in logical:
        if 'px_root_push()' in content and 'PX_KEEP(' in content:
            v.append((ln, '同一语句行内 `px_root_push()` 与 `PX_KEEP(` 共现', content.strip()[:120]))

    # 规则 2：相邻两行 = 两段式（剥注释后逐行判，容忍缩进）
    for i in range(len(stripped) - 1):
        if re.fullmatch(r'px_root_push\(\);', stripped[i]) and stripped[i + 1].startswith('PX_KEEP('):
            v.append((i + 1, '相邻两段式 px_root_push(); + PX_KEEP(', stripped[i + 1][:120]))
    return v


def self_test():
    ok = True
    cases = [
        ("两段式（无注释）", "a;\n    px_root_push();\n    PX_KEEP(x);\n", 1),
        ("两段式（行尾注释 —— M208 门漏判的形态）",
         "a;\n    px_root_push();   // 说明\n    PX_KEEP(x);\n", 1),
        ("两段式（PX_KEEP 行尾注释）",
         "a;\n    px_root_push();\n    PX_KEEP(x);   // 说明\n", 1),
        ("同一行两段式（宏续行）",
         "    px_root_push(); /* c */ \\\n    LXValue a = f(); PX_KEEP(a); \\\n", 1),
        ("注释里提到两者（不得误报）",
         "    // px_root_push(); 与 PX_KEEP(x) 的旧写法\n    px_root_push_keep(x);\n", 0),
        ("已改写的原子原语（合规）", "    px_root_push_keep(x);\n", 0),
    ]
    import tempfile
    for name, body, want in cases:
        fd, p = tempfile.mkstemp(suffix='.c')
        os.write(fd, body.encode())
        os.close(fd)
        got = len(violations(p))
        os.unlink(p)
        flag = '✅' if got == want else '❌'
        if got != want:
            ok = False
        print("   %s 自证 %-40s 违例 %d（期望 %d）" % (flag, name, got, want))
    return 0 if ok else 2


def main():
    args = sys.argv[1:]
    if '--self-test' in args:
        return self_test()
    files = []
    if '--files' in args:
        files = args[args.index('--files') + 1:]
    if not files:
        files = sorted(glob.glob('runtime/*.c'))
    total = 0
    for p in files:
        v = violations(p)
        for ln, why, ctx in v:
            print("   ❌ %s:%d  %s  ::  %s" % (p, ln, why, ctx))
            total += 1
    if total == 0:
        print("   ✅ S13 不变量：%d 个文件 · 无违例（注释容忍 + 续行合并）" % len(files))
        return 0
    print("   ❌ S13 违例 %d 处" % total)
    return 1


if __name__ == '__main__':
    sys.exit(main())
