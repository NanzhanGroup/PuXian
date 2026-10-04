#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
M256 静态守卫：EINTR 族（「被 GC 暂停信号打断的 IO 不是失败」）

A 段（无例外族）：poll / select / epoll_wait / nanosleep / usleep / sigsuspend
  —— 这些接口**永不**被 SA_RESTART 重启（signal(7) 明确列举），无论 fd 有没有超时。
B 段（用户面 fd 原语）：所有 `bi_*` native 体内的 read/write/recv/send/recvfrom/sendto/accept/connect
  —— 用户面 fd（tcp_connect_ex/tcp_opt/px_serve 的连接）**都设了 SO_*TIMEO**
  ⇒ 同样落在「永不重启」族。

每个站点必须满足其一（否则判红）：
  ① 已经是 `px_io_*` 包装调用；
  ② 调用点 ±WIN 行窗口内有 `EINTR` 判据（同语句/循环内）；
  ③ 该行带 `PX_IO_EINTR_OK` 豁免标记（**必须附理由**）。

用法：
  check_eintr.py [仓库根]      # 严格检查（默认根 = 脚本所在目录的上一级）
  check_eintr.py --self-test   # 判据自证（fixture 全在临时目录，与真实仓库无关）

为什么需要它：M211 缺陷 265、M152 缺陷 146、M256 缺陷 456 是**同一形状**反复出现 ——
  「某处的裸 IO 忘了重试 EINTR」。修一次只覆盖一处；本守卫让**新增**的裸 IO 当场判红。
"""
import io
import os
import re
import shutil
import sys
import tempfile

FAMILY_A = ['poll', 'select', 'epoll_wait', 'nanosleep', 'usleep', 'sigsuspend', 'sigtimedwait']
FAMILY_B = ['read', 'write', 'recv', 'send', 'recvfrom', 'sendto', 'accept', 'accept4', 'connect']
WIN = 8
WRAPPER_SIG = 'px_io_'
MIN_C_FILES = 12          # 规模锚点：防「扫不到文件 ⇒ 静默判绿」
MIN_WRAPPED = 30          # 规模锚点：防「包装调用全没了 ⇒ 空集 ⊇ 任意集」

A_RE = re.compile(r'(?<![\w.>])(%s)\s*\(' % '|'.join(FAMILY_A))
B_RE = re.compile(r'(?<![\w.>])(%s)\s*\(' % '|'.join(FAMILY_B))
FN_RE = re.compile(r'\b(bi_[A-Za-z0-9_]+)\s*\([^;]*\)\s*\{')


def strip_code(line):
    s = re.sub(r'/\*.*?\*/', ' ', line)
    s = re.sub(r'//.*$', ' ', s)
    s = re.sub(r'"(?:[^"\\]|\\.)*"', '""', s)
    s = re.sub(r"'(?:[^'\\]|\\.)*'", "''", s)
    return s


def strip_block_comments(lines):
    out, inblk = [], False
    for s in lines:
        if inblk:
            e = s.find('*/')
            if e < 0:
                out.append(' ')
                continue
            s = s[e + 2:]
            inblk = False
        while True:
            b = s.find('/*')
            if b < 0:
                break
            e = s.find('*/', b + 2)
            if e < 0:
                s, inblk = s[:b], True
                break
            s = s[:b] + ' ' + s[e + 2:]
        out.append(s)
    return out


def exempt(raw, code, i):
    if 'PX_IO_EINTR_OK' in raw[i]:
        return True
    lo, hi = max(0, i - WIN), min(len(code), i + WIN + 1)
    return 'EINTR' in '\n'.join(code[lo:hi])


def scan_file(path):
    raw = io.open(path, encoding='utf-8', errors='replace').read().split('\n')
    code = [strip_code(x) for x in strip_block_comments(raw)]
    va, vb, nwrap = [], [], 0
    for i, line in enumerate(code):
        nwrap += line.count(WRAPPER_SIG)
        for m in A_RE.finditer(line):
            if not exempt(raw, code, i):
                va.append((i + 1, m.group(1), raw[i].strip()[:110]))
    for i, line in enumerate(code):
        m = FN_RE.search(line)
        if not m:
            continue
        fn, depth = m.group(1), 0
        for j in range(i, min(len(code), i + 600)):
            depth += code[j].count('{') - code[j].count('}')
            if j > i or depth > 0:
                for mm in B_RE.finditer(code[j]):
                    if not exempt(raw, code, j):
                        vb.append((j + 1, fn, mm.group(1), raw[j].strip()[:110]))
            if depth == 0 and j > i:
                break
    return va, vb, nwrap


def scan_root(root):
    rt = os.path.join(root, 'runtime')
    if not os.path.isdir(rt):
        return None
    files = [f for f in sorted(os.listdir(rt)) if f.endswith('.c')]
    va_all, vb_all, nwrap = [], [], 0
    for f in files:
        va, vb, nw = scan_file(os.path.join(rt, f))
        nwrap += nw
        for ln, name, txt in va:
            va_all.append((f, ln, name, txt))
        for ln, fn, name, txt in vb:
            vb_all.append((f, ln, fn, name, txt))
    return files, va_all, vb_all, nwrap


# ── 自证 fixture：(名称, 源, 期望 A 违例数, 期望 B 违例数) ──
SELF_TESTS = [
    ('A1 裸 poll（无 EINTR）',
     'void f(void) {\n    int r = poll(p, 1, 0);\n    (void)r;\n}\n', 1, 0),
    ('A2 poll 带 EINTR 循环',
     'void f(void) {\n    int r;\n    do { r = poll(p, 1, 10); } while (r < 0 && errno == EINTR);\n}\n', 0, 0),
    ('A3 poll 后 2 行内 EINTR 判据',
     'void f(void) {\n    int r = poll(p, 1, 10);\n    if (r < 0) { if (errno == EINTR) return; }\n}\n', 0, 0),
    ('A4 nanosleep 带豁免标记',
     'void f(void) {\n    nanosleep(&ts, NULL);   /* PX_IO_EINTR_OK: futex 语义，提前返回由调用方复检 */\n}\n', 0, 0),
    ('A5 已走包装',
     'void f(void) {\n    px_io_poll(p, 1, 0);\n    px_io_sleep_ms(5);\n}\n', 0, 0),
    ('B1 bi_read 内裸 read',
     'static LXValue bi_read(LXValue* a, int n, void* c) {\n'
     '    ssize_t k = read(fd, buf, 8);\n    return px_null();\n}\n', 0, 1),
    ('B2 bi_read 内带 EINTR 重试',
     'static LXValue bi_read(LXValue* a, int n, void* c) {\n'
     '    for (;;) { ssize_t k = read(fd, buf, 8); if (k >= 0 || errno != EINTR) break; }\n'
     '    return px_null();\n}\n', 0, 0),
    ('B3 bi_udp_recv 内裸 recvfrom',
     'static LXValue bi_udp_recv(LXValue* a, int n, void* c) {\n'
     '    int k = recvfrom(fd, b, m, 0, &s, &l);\n    return px_null();\n}\n', 0, 1),
    ('B4 非 bi_ 函数不属 B 段',
     'static int helper(int fd) {\n    return (int)read(fd, b, 8);\n}\n', 0, 0),
]


def self_test():
    ok = bad = 0
    tmp = tempfile.mkdtemp(prefix='px_eintr_selftest.')
    try:
        os.makedirs(os.path.join(tmp, 'runtime'))
        for name, src, ea, eb in SELF_TESTS:
            p = os.path.join(tmp, 'runtime', 'x.c')
            io.open(p, 'w', encoding='utf-8').write(src)
            va, vb, _ = scan_file(p)
            got = (len(va), len(vb))
            if got == (ea, eb):
                print('  PASS %-32s A=%d B=%d' % (name, got[0], got[1]))
                ok += 1
            else:
                print('  FAIL %-32s 期望 A=%d B=%d 实测 A=%d B=%d'
                      % (name, ea, eb, got[0], got[1]))
                bad += 1
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    print('自证：通过 %d / 失败 %d' % (ok, bad))
    return 1 if bad else 0


def main():
    if '--self-test' in sys.argv:
        return self_test()
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    root = args[0] if args else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    r = scan_root(root)
    if r is None:
        print('❌ 找不到 %s/runtime 目录（拒绝静默判绿）' % root)
        return 3
    files, va, vb, nwrap = r
    for f, ln, name, txt in va:
        print('A| %-18s :%-6d %-12s | %s' % (f, ln, name, txt))
    for f, ln, fn, name, txt in vb:
        print('B| %-18s :%-6d %-18s %-10s | %s' % (f, ln, fn, name, txt))
    print('扫描 %d 个 .c · 包装调用 %d 处' % (len(files), nwrap))
    bad = 0
    if len(files) < MIN_C_FILES:
        print('❌ 规模锚点：扫到 %d 个 .c（下限 %d）⇒ 判据可能失效' % (len(files), MIN_C_FILES))
        bad = 1
    if nwrap < MIN_WRAPPED:
        print('❌ 规模锚点：包装调用 %d 处（下限 %d）⇒ 空集 ⊇ 任意集' % (nwrap, MIN_WRAPPED))
        bad = 1
    if va or vb:
        print('❌ A 段 %d 处 · B 段 %d 处（须为 0；豁免须带 PX_IO_EINTR_OK 标记与理由）'
              % (len(va), len(vb)))
        bad = 1
    if bad:
        return 1
    print('✅ EINTR-GUARD-OK：A 段 0 · B 段 0（裸 IO 全部经 px_io_* 或就地重试）')
    return 0


if __name__ == '__main__':
    sys.exit(main())
