#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""check_gc_lock_order.py —— GC 暂停信号 / 互斥锁顺序守卫（M312 建立）

判据（唯一）：
    `gc_unblock_stop(&old);` 之后**紧跟**（中间只允许空行/注释）`pthread_mutex_unlock(&X);`
    ⇒ **判红**。正确写法是「先放锁、再解除信号屏蔽」。

为什么（缺陷 535 实测）：
    `gc_block_stop()` 期间 SIG_GC_STOP 是 pending。`gc_unblock_stop()` 一执行，pending 信号
    立即递达 ⇒ 若此刻仍持锁，线程**持着锁**进暂停处理器；而 GC executor 随后在
    `*_gc_mark()` 里等这把锁 ⇒ 整条服务被按住，直到被暂停线程的 handler 兜底（300×20ms≈6s，
    负载下实测 12s）。生产现场（晨曦 Mahesvara）：p95 12822ms / max 12933ms；
    本机最小复现 12146ms，修后 34ms。

用法：
    check_gc_lock_order.py [--root DIR] [--self-test]
退出码：0 = 无违例；1 = 有违例；3 = 判据自证失败 / 扫描面异常（判定不了不许放行）
"""
import os
import re
import sys

# 只看代码行：注释与字符串里的示例**不算**（M200/M251 教训：剥离要保留字面量、去掉注释）
def strip_comments(src):
    """去注释 + **屏蔽字符串内容**（保留引号与换行，内容换空格）。

    ⚠️ 为什么要屏蔽字符串：本判据只看「代码结构」，而字面量里出现
    `"... gc_unblock_stop(&old); pthread_mutex_unlock(&m); ..."` 是**文本**不是代码
    ⇒ 不屏蔽会误报（自证用例「字符串字面量里的反序」当场抓到）。
    与 M200「提取注册名时要保留字面量」相反 —— **判据用途不同，剥离方式就不同**。
    """
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if c == '"' or c == "'":
            q = c
            out.append(c)
            i += 1
            while i < n:
                if src[i] == '\\' and i + 1 < n:
                    out.append('  ')
                    i += 2
                    continue
                if src[i] == q:
                    out.append(q)
                    i += 1
                    break
                out.append('\n' if src[i] == '\n' else ' ')
                i += 1
            continue
        if c == '/' and i + 1 < n and src[i + 1] == '*':
            j = src.find('*/', i + 2)
            seg = src[i:(j + 2 if j >= 0 else n)]
            out.append('\n' * seg.count('\n'))
            i = (j + 2 if j >= 0 else n)
            continue
        if c == '/' and i + 1 < n and src[i + 1] == '/':
            j = src.find('\n', i)
            i = (j if j >= 0 else n)
            continue
        out.append(c)
        i += 1
    return ''.join(out)

UNBLOCK = re.compile(r'gc_unblock_stop\s*\(\s*&?\w+\s*\)\s*;')
UNLOCK = re.compile(r'pthread_(?:mutex|rwlock)_unlock\s*\(\s*&\w+\s*\)\s*;')

def scan_text(name, src):
    """返回 [(行号, 说明)]。判据：unblock 之后第一句代码是 unlock ⇒ 反序。"""
    code = strip_comments(src)
    hits = []
    for m in UNBLOCK.finditer(code):
        tail = code[m.end():]
        # 只允许空行/空白，取下一个非空语句
        rest = tail.lstrip()
        if not rest:
            continue
        # 取到下一个分号为止
        semi = rest.find(';')
        if semi < 0:
            continue
        stmt = rest[:semi + 1].strip()
        if UNLOCK.match(stmt):
            line = code[:m.start()].count('\n') + 1
            hits.append((line, stmt))
    return hits

def collect(root):
    files = []
    rt = os.path.join(root, 'runtime')
    for dirpath, _dirs, names in os.walk(rt):
        for nm in names:
            if nm.endswith('.c') or nm.endswith('.h'):
                files.append(os.path.join(dirpath, nm))
    files.sort()
    return files

def main():
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
    selftest = False
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == '--root' and i + 1 < len(args):
            root = args[i + 1]; i += 2; continue
        if args[i] == '--self-test':
            selftest = True; i += 1; continue
        sys.stderr.write('未知参数: %s\n' % args[i]); sys.exit(2)

    if selftest:
        sys.exit(run_selftest())

    files = collect(root)
    if len(files) < 20:
        sys.stderr.write('扫描面异常：只找到 %d 个 runtime 源文件（判定不了 ⇒ 拒绝放行）\n' % len(files))
        sys.exit(3)
    total = 0
    for f in files:
        try:
            src = open(f, encoding='utf-8', errors='replace').read()
        except OSError:
            continue
        hits = scan_text(f, src)
        if hits:
            total += len(hits)
            rel = os.path.relpath(f, root)
            for line, stmt in hits:
                print('[violation] %s:%d  unblock 之后紧接 %s' % (rel, line, stmt))
    print('扫描 %d 个文件 · 反序违例 %d 处' % (len(files), total))
    sys.exit(1 if total else 0)

def run_selftest():
    cases = [
        # (名称, 源码, 期望命中数)
        ('反序（应判红）', 'void f(void){\n  gc_unblock_stop(&old);\n  pthread_mutex_unlock(&g_mu);\n}\n', 1),
        ('正序（应放行）', 'void f(void){\n  pthread_mutex_unlock(&g_mu);\n  gc_unblock_stop(&old);\n}\n', 0),
        ('注释里的反序（不算）', '// gc_unblock_stop(&old);\n// pthread_mutex_unlock(&g_mu);\nvoid f(void){}\n', 0),
        ('块注释里的反序（不算）', '/*\n gc_unblock_stop(&old);\n pthread_mutex_unlock(&g_mu);\n*/\nvoid f(void){}\n', 0),
        ('中间隔了别的语句（不算紧邻）', 'void f(void){\n  gc_unblock_stop(&old);\n  x = 1;\n  pthread_mutex_unlock(&g_mu);\n}\n', 0),
        ('字符串字面量里的反序（不算）', 'const char* s = "gc_unblock_stop(&old); pthread_mutex_unlock(&g_mu);";\n', 0),
        ('rwlock 反序（应判红）', 'void f(void){\n  gc_unblock_stop(&old);\n  pthread_rwlock_unlock(&g_rw);\n}\n', 1),
        ('多行缩进+空行（应判红）', 'void f(void){\n    gc_unblock_stop(&old);\n\n\n    pthread_mutex_unlock(&g_mu);\n}\n', 1),
    ]
    ok = 0
    fail = 0
    for name, src, want in cases:
        got = len(scan_text('x', src))
        if got == want:
            ok += 1
            print('  ✅ 自证 %s（命中 %d）' % (name, got))
        else:
            fail += 1
            print('  ❌ 自证 %s：期望 %d，实际 %d' % (name, want, got))
    # 判据自伤负控：把 UNLOCK 改成永不匹配 ⇒ 「反序」用例必须不再命中
    global UNLOCK
    saved = UNLOCK
    UNLOCK = re.compile(r'NEVER_MATCH_XYZ')
    got = len(scan_text('x', cases[0][1]))
    UNLOCK = saved
    if got == 0:
        ok += 1
        print('  ✅ 自证 判据自伤（关掉判据 ⇒ 反序不再被识别）')
    else:
        fail += 1
        print('  ❌ 自证 判据自伤：仍命中 %d（说明判据不依赖于 UNLOCK 规则）' % got)
    print('SELFTEST %s 通过 %d 失败 %d' % ('OK' if fail == 0 else 'FAIL', ok, fail))
    return 0 if fail == 0 else 3

if __name__ == '__main__':
    main()
