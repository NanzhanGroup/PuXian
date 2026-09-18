#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""check_whitelist.py — 静态核验 runtime 的「是否为堆对象」类型表（M134 / 缺陷 86）

为什么要有它：缺陷 86 的根因是 `px_value_is_obj` 的 switch 白名单**静默漏项**
（漏 PX_MUTEX / PX_RWLOCK / PX_GEN）⇒ 这三种值进容器/全局/帧时不被 GC 标记 → 悬垂。
C 侧已改为 `g_type_is_obj[PX_TYPE_MAX]` 位置表 + `_Static_assert` 尺寸断言（编译期
保证"LXType 新增成员必须表态"）。本脚本是**独立的第二道核对**：不看编译产物，直接读
源码文本，逐条核对「枚举成员全集 ↔ 表条目」，并断言三种锁/生成器必须为 true。

用法： check_whitelist.py <runtime.c> <runtime.h>
退出码：0 = 通过；1 = 不通过（打印原因）
"""
import re
import sys


def parse_enum(hdr_text):
    m = re.search(r'typedef enum\s*\{(.*?)\}\s*LXType\s*;', hdr_text, re.S)
    if not m:
        raise SystemExit("❌ 未在 runtime.h 找到 `typedef enum { ... } LXType;`")
    body = m.group(1)
    body = re.sub(r'//[^\n]*', '', body)          # 去行注释
    members = []
    for part in body.split(','):
        part = part.strip()
        if not part:
            continue
        name = part.split('=')[0].strip()
        if re.fullmatch(r'[A-Z_][A-Z0-9_]*', name):
            members.append(name)
    return members


def parse_table(c_text):
    m = re.search(r'static const bool g_type_is_obj\[PX_TYPE_MAX\]\s*=\s*\{(.*?)\};',
                  c_text, re.S)
    if not m:
        raise SystemExit("❌ 未在 runtime.c 找到 `g_type_is_obj[PX_TYPE_MAX]` 表")
    body = m.group(1)
    entries = []
    for name, val in re.findall(r'/\*\s*([A-Z_][A-Z0-9_]*)\s*\*/\s*(true|false)', body):
        entries.append((name, val == 'true'))
    return entries


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    c_text = open(sys.argv[1], encoding='utf-8').read()
    h_text = open(sys.argv[2], encoding='utf-8').read()

    members = parse_enum(h_text)
    entries = parse_table(c_text)
    bad = []

    # 哨兵必须是最后一个枚举成员
    if not members or members[-1] != 'PX_TYPE_MAX':
        bad.append("LXType 最后一个成员应为哨兵 PX_TYPE_MAX（尺寸断言的基准）")
    names = [n for n, _ in entries]

    # ① 表必须与枚举成员**一一对应、同序**（漏项/多项/错序都判红）
    expect = members[:-1] if members and members[-1] == 'PX_TYPE_MAX' else members
    if names != expect:
        missing = [n for n in expect if n not in names]
        extra = [n for n in names if n not in expect]
        if missing:
            bad.append("表漏项（未表态）：" + ", ".join(missing))
        if extra:
            bad.append("表含未知项：" + ", ".join(extra))
        if not missing and not extra:
            bad.append("表项顺序与 LXType 声明序不一致：\n      表 = %s\n      枚举 = %s"
                       % (" ".join(names), " ".join(expect)))

    # ② 三种「曾被漏标」的类型必须为 true（缺陷 86 的直接防线）
    d = dict(entries)
    for t in ("PX_MUTEX", "PX_RWLOCK", "PX_GEN"):
        if t not in d:
            bad.append("%s 不在表中（缺陷 86 复发）" % t)
        elif not d[t]:
            bad.append("%s 被标为 false ⇒ 该类型的值进容器/全局/帧后会被 GC 误回收" % t)

    # ③ 纯值类型必须为 false（防"全 true"式偷懒修法：那会锁住不可回收的字面量）
    for t in ("PX_NULL", "PX_BOOL", "PX_INT", "PX_FLOAT"):
        if d.get(t, True):
            bad.append("%s 应为 false（纯值类型不持有堆对象）" % t)

    if bad:
        print("❌ 类型表核验不通过：")
        for b in bad:
            print("   · " + b)
        return 1
    print("✅ 类型表核验通过：%d 项 = LXType 成员数（含哨兵 %d），MUTEX/RWLOCK/GEN 均为 true"
          % (len(entries), len(entries) + 1))
    return 0


if __name__ == '__main__':
    sys.exit(main())
