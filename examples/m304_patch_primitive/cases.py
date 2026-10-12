#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M304 门语料 —— 「打桩原语」的差分用例表（**单一事实源**）。

为什么要独立成表：M302 的 [E] 层教训 —— 「用例表与驱动器脱节 ⇒ 静默不执行」。
本文件同时被 diffcheck.py（驱动）与 verify.sh（规模锚点）引用：
驱动器逐条执行；verify.sh 只断言「表里条数 == 驱动器报出的条数」，脱节即判红。

每例 = (名称, 初始文件内容, 操作列表)。操作字段：old / new / count。
  · count 是**语义**字段（不绑任何工具的参数名）：
      patch_file 侧 → --count N ; wst-edit 侧 → expect=N
    ⇒ 两侧参数名不同但语义必须一致，这正是差分要证的事。
"""
CASES = [
    ("C1 纯文本", "alpha foo omega\n", [{"old": "foo", "new": "bar", "count": 1}]),
    ("C2 危险 &", "alpha TOK omega\n", [{"old": "TOK", "new": "X&Y", "count": 1}]),
    ("C3 危险 \\u", 'r = "TOK";\n', [{"old": "TOK", "new": "\\u", "count": 1}]),
    ("C4 危险 $ \\1", "v = TOK;\n", [{"old": "TOK", "new": "$1 \\1 $HOME", "count": 1}]),
    ("C5 分隔符 |", "kT\n", [{"old": "kT", "new": "a|b", "count": 1}]),
    ("C6 反斜杠路径", "p = TOK\n", [{"old": "TOK", "new": "C:\\path\\to\\x", "count": 1}]),
    ("C7 无命中（必须响亮）", "hello\n", [{"old": "NOTPRESENT", "new": "x", "count": 1}]),
    ("C8 多处命中 count=1", "aXbXc\n", [{"old": "X", "new": "-", "count": 1}]),
    ("C9 多处命中 count=2", "aXbXc\n", [{"old": "X", "new": "-", "count": 2}]),
    ("C10 多行匹配", "L1\nL2\nL3\n", [{"old": "L1\nL2", "new": "M1\nM2", "count": 1}]),
    ("C11 权限保留", "TOK\n", [{"old": "TOK", "new": "tok", "count": 1}]),
    ("C12 原子性（第 2 处失败）", "A1\nB1\n",
     [{"old": "A1", "new": "A2", "count": 1}, {"old": "ZZZ", "new": "y", "count": 1}]),
]

# sed 对照层只对「单处替换」有意义（批量不属于 sed 的用法）——显式计数，别静默少测
SED_APPLICABLE = [c for c in CASES if len(c[2]) == 1]
