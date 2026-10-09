#!/usr/bin/env python3
# ============================================================
# examples/m294_h3_qpack_share/cstrip.py —— C 源码「去注释」器
# ------------------------------------------------------------
# 为什么需要（本仓反复踩过的形状）：
#   M234 缺陷 352 的注释里写了标识符名字 ⇒ 静态判据读到**注释里的同名文本**（假红/假绿）；
#   M286 的 A1/A4「打桩锚点在位 / 唯一性」判据同样必须先去掉散文，否则「解释这句注释」
#   与「代码真的这么写」不可区分。
#
# 判据（**只去注释、保留字面量**）：
#   · 块注释 `/* … */`（含跨行、未闭合则吃到文件尾）
#   · 行注释 `// …` 到行尾
#   · 字符串 `"…"` 与字符 `'…'` **整体保留**（含 `\\` 转义），不把里面的 `//` 当注释
#   ⚠️ 不追求完整 C 词法（不展开宏、不处理行拼接续行）—— 本门只用它做**标识符形态**判据。
#
# 用法：cstrip.py <file.c>   → stdout
# ============================================================
import sys


def strip(src):
    out = []
    i = 0
    n = len(src)
    while i < n:
        c = src[i]
        if c == '"' or c == "'":
            q = c
            out.append(c)
            i += 1
            while i < n:
                if src[i] == "\\" and i + 1 < n:
                    out.append(src[i:i + 2])
                    i += 2
                    continue
                out.append(src[i])
                if src[i] == q:
                    i += 1
                    break
                i += 1
            continue
        if c == "/" and i + 1 < n and src[i + 1] == "*":
            j = src.find("*/", i + 2)
            i = (j + 2) if j >= 0 else n
            continue
        if c == "/" and i + 1 < n and src[i + 1] == "/":
            j = src.find("\n", i)
            i = j if j >= 0 else n
            continue
        out.append(c)
        i += 1
    return "".join(out)


def main():
    if len(sys.argv) != 2:
        print("用法: cstrip.py <file.c|->   （`-` = 读 stdin）", file=sys.stderr)
        return 2
    if sys.argv[1] == "-":
        # ⚠️ stdin 模式存在的理由：门的自证若用 `importlib` 加载本模块，会在门目录里
        #    生成 `__pycache__/` ⇒ **污染仓库**（全量门会判「工作树不干净」）。
        #    用子进程 + stdin 就没有这个问题。
        src = sys.stdin.read()
    else:
        with open(sys.argv[1], encoding="utf-8", errors="replace") as f:
            src = f.read()
    sys.stdout.write(strip(src))
    return 0


if __name__ == "__main__":
    sys.exit(main())
