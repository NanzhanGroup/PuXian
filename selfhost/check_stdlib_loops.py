#!/usr/bin/env python3
# ============================================================
# M279 · stdlib「结构性死循环」静态扫描器
# ------------------------------------------------------------
# 由来（真缺陷 · 高危）：
#   `std.collections.chunk(items, n)` 的内层 `while j < n and i < len(items)`
#   在 `n <= 0` 时**恒假** ⇒ 循环体（`i += 1` 所在的唯一位置）永不执行
#   ⇒ 外层 `while i < len(items)` 的条件**永不变化** ⇒ **死循环 + result 无限增长**。
#   实测：解释轨挂住 60s；限内存 300MB 时报「内存不足 / 分配尺寸非法」——
#   **错误信息指不到真因**（用户看不到"你传了 0"）。
#
# 判据（本扫描器）：
#   对每个 `while` 循环，取其条件里出现的**变量集 V**；检查 V 在循环体内的赋值位置：
#     · V 在**循环体最外层**被赋值            ⇒ OK（每轮推进）
#     · V 只在**嵌套块内**被赋值              ⇒ **MEDIUM**（内层条件可能恒假 ⇒ 死循环）
#     · V 在循环体内**完全没有**被赋值        ⇒ **HIGH**（除非 break/return）
#   ⚠️ 缩进语言（普贤）：块 = 缩进增加；"最外层"= 比循环行缩进大的**最小**缩进级。
#   ⚠️ `for` 循环不扫（迭代器天然推进）。
#
# 退出码：0 = 无违例；1 = 有违例；2 = 环境错误
# 用法：check_stdlib_loops.py [--root .] [--self-test] [--json] [--allow FILE]
# ============================================================
import argparse, json, os, re, sys

KW = {'while', 'if', 'else', 'for', 'in', 'and', 'or', 'not', 'return', 'break',
      'continue', 'def', 'var', 'let', 'const', 'true', 'false', 'null', 'type',
      'len', 'range', 'import', 'from', 'as', 'match', 'case', 'default', 'elif'}
ID_RX = re.compile(r'[A-Za-z_][A-Za-z0-9_]*')
ASSIGN_RX = re.compile(r'^\s*([A-Za-z_][A-Za-z0-9_]*)\s*(=[^=]|\+=|-=|\*=|/=|%=)')


def indent_of(line):
    return len(line) - len(line.lstrip(' '))


def strip_comment(line):
    """去 `#` 注释（**保留字符串字面量内的 #**：简易处理，成对引号内不切）"""
    out, q = [], None
    for ch in line:
        if q:
            out.append(ch)
            if ch == q:
                q = None
        elif ch in '"\'':
            q = ch; out.append(ch)
        elif ch == '#':
            break
        else:
            out.append(ch)
    return ''.join(out)


def cond_vars(cond):
    """条件里的标识符（去掉关键字/数字/字符串字面量）"""
    s = re.sub(r'"[^"]*"', ' ', cond)
    s = re.sub(r"'[^']*'", ' ', s)
    return {m.group(0) for m in ID_RX.finditer(s)} - KW


_HARM = False


def scan_source(text, fname):
    """返回违例列表 [{file,line,var,level,kind}]"""
    if _HARM:            # 负控 C：判据自伤（恒不报）
        return []
    lines = [strip_comment(l) for l in text.split('\n')]
    n = len(lines)
    out = []
    # 全文件「被赋值过的名字」—— 用于把 HIGH 的报出面收窄到**计数器类**局部变量
    # （`while i < len(xs)` 里 `xs` 是数据、`i` 是计数器；只有后者才可能"该推进却没推进"）。
    assigned_any = set()
    for l in lines:
        am = ASSIGN_RX.match(l)
        if am:
            assigned_any.add(am.group(1))
    for i, ln in enumerate(lines):
        m = re.match(r'^(\s*)while\b(.*?):\s*$', ln)
        if not m:
            continue
        base = len(m.group(1))
        cond = m.group(2)
        vars_ = cond_vars(cond)
        if not vars_:
            continue
        # 只关心「**裸标识符**」形式的条件变量（后不跟 `(`）—— 函数调用型
        # （`while not queue_empty():`）静态判不出推进，跳过以免假阳。
        bare = {v for v in vars_ if not re.search(r'\b%s\s*\(' % re.escape(v), cond)}
        # 收集循环体（缩进 > base）
        body, j = [], i + 1
        while j < n:
            l2 = lines[j]
            if l2.strip() == '':
                body.append((j, l2)); j += 1; continue
            if indent_of(l2) <= base:
                break
            body.append((j, l2)); j += 1
        if not body:
            continue
        min_ind = min(indent_of(l2) for _, l2 in body if l2.strip())
        direct, nested = set(), set()
        for k, l2 in body:
            am = ASSIGN_RX.match(l2)
            if not am:
                continue
            v = am.group(1)
            if indent_of(l2) == min_ind:
                direct.add(v)
            else:
                nested.add(v)
        # 循环体内的**内层循环**行（缩进 > base 的 while/for）—— 只有「推进语句落在
        # 内层循环体内」才危险（内层条件可能恒假）；落在 if/else 条件块内是安全的
        # （分支穷尽 ⇒ 每轮必推进），实测 `count_of`/`split` 等大量真实代码属后者 ⇒ 不报。
        inner_loops = [(k, indent_of(l2)) for k, l2 in body
                       if re.match(r'^\s*(while|for)\b', l2)]

        def _in_inner(k):
            ind = indent_of(lines[k])
            return any(kl < k and ind > li for kl, li in inner_loops)

        touched = bare & (direct | nested)      # 条件变量 ∩ 体内被赋值变量
        if touched:
            missing = touched - direct          # 未在循环体最外层推进的
            for v in sorted(missing):
                rows = []
                for k, l2 in body:
                    am2 = ASSIGN_RX.match(l2)
                    if am2 and am2.group(1) == v and indent_of(l2) != min_ind and _in_inner(k):
                        rows.append(k)
                if rows:
                    out.append({'file': fname, 'line': i + 1, 'var': v,
                                'level': 'MEDIUM',
                                'kind': '推进只在内层循环体内（内层条件可能恒假）'})
        else:
            # 条件变量在体内**完全没被赋值** ⇒ 唯一出口是 break/return/panic
            has_exit = any(re.search(r'\b(break|return|panic)\b', l2) for _, l2 in body)
            if not has_exit:
                for v in sorted(bare & assigned_any):
                    out.append({'file': fname, 'line': i + 1, 'var': v,
                                'level': 'HIGH', 'kind': '条件变量体内无赋值且无出口'})
    return out


# ---- 自证 fixture：4 正 + 3 负 ----
FIX_BAD_INNER = '''def chunk(items, n):
    result = []
    i = 0
    while i < len(items):
        part = []
        j = 0
        while j < n and i < len(items):
            part.append(items[i])
            i = i + 1
            j = j + 1
        result.append(part)
    return result
'''
FIX_BAD_NONE = '''def f(xs):
    i = 0
    while i < len(xs):
        print("loop")
    return 1
'''
FIX_OK_DIRECT = '''def f(xs):
    i = 0
    while i < len(xs):
        print(xs[i])
        i = i + 1
    return 1
'''
FIX_OK_FOR = '''def f(xs):
    for x in xs:
        print(x)
    return 1
'''
FIX_OK_WHILE_TRUE = '''def f(xs):
    while true:
        break
    return 1
'''


def self_test():
    ok = bad = 0
    def chk(name, got, want):
        nonlocal ok, bad
        if got == want:
            print('  ✅ %s' % name); ok += 1
        else:
            print('  ❌ %s（got=%r want=%r）' % (name, got, want)); bad += 1

    r = scan_source(FIX_BAD_INNER, 'fix')
    chk('① chunk 形状必须报（MEDIUM）', len(r) == 1 and r[0]['level'] == 'MEDIUM', True)
    chk('① 变量名 = i', r and r[0]['var'] == 'i', True)
    r = scan_source(FIX_BAD_NONE, 'fix')
    chk('② 体内无赋值必须报（HIGH）', len(r) == 1 and r[0]['level'] == 'HIGH', True)
    chk('③ 最外层推进 ⇒ 不报', len(scan_source(FIX_OK_DIRECT, 'fix')) == 0, True)
    chk('④ for 循环不扫', len(scan_source(FIX_OK_FOR, 'fix')) == 0, True)
    chk('⑤ while true + break ⇒ 条件无变量 ⇒ 不报',
        len(scan_source(FIX_OK_WHILE_TRUE, 'fix')) == 0, True)
    # ⑥ 负控：把判据改成「恒不报」⇒ ①② 必须失守（证明判据有牙）
    global _HARM
    _HARM = True
    chk('⑥ 判据自伤：恒不报 ⇒ ①形状漏报', len(scan_source(FIX_BAD_INNER, 'fix')) == 0, True)
    _HARM = False
    chk('⑦ 恢复后 ① 重新报出', len(scan_source(FIX_BAD_INNER, 'fix')) == 1, True)
    print('\n══ 自证：%d 通过 / %d 失败 ══' % (ok, bad))
    return 1 if bad else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default='.')
    ap.add_argument('--self-test', action='store_true')
    ap.add_argument('--json', default=None)
    ap.add_argument('--allow', default=None, help='豁免表（每行 <文件>:<行> <理由>，理由为空无效）')
    a = ap.parse_args()
    if a.self_test:
        return self_test()

    allow = {}
    if a.allow and os.path.exists(a.allow):
        for ln in open(a.allow, encoding='utf-8'):
            if ln.startswith('#') or not ln.strip():
                continue
            parts = ln.split(None, 1)
            if len(parts) == 2 and parts[1].strip():
                allow[parts[0]] = parts[1].strip()

    files = []
    d = os.path.join(a.root, 'stdlib')
    for f in sorted(os.listdir(d)):
        if f.endswith('.px'):
            files.append(os.path.join('stdlib', f))

    allv = []
    for rel in files:
        p = os.path.join(a.root, rel)
        try:
            txt = open(p, encoding='utf-8', errors='replace').read()
        except Exception:
            continue
        allv.extend(scan_source(txt, rel))

    kept, waived, stale = [], 0, []
    for v in allv:
        key = '%s:%d' % (v['file'], v['line'])
        if key in allow:
            waived += 1; v['waived'] = allow[key]
        else:
            kept.append(v)
    for key in allow:
        if not any(('%s:%d' % (v['file'], v['line'])) == key for v in allv):
            stale.append(key)

    print('══ M279 stdlib 结构性死循环扫描 ══')
    print('  文件 %d · 违例 %d · 豁免 %d · 豁免过期 %d' % (len(files), len(kept), waived, len(stale)))
    for v in kept:
        print('  ❌ %s:%d [%s] 变量 %s —— %s' % (v['file'], v['line'], v['level'], v['var'], v['kind']))
    for k in stale:
        print('  ⚠️ 豁免过期：%s（源码里已无该违例）' % k)
    if a.json:
        json.dump({'violations': kept, 'waived': waived, 'stale': stale},
                  open(a.json, 'w'), ensure_ascii=False, indent=1)
    rc = 0
    if kept:
        print('❌ M279-LOOPSCAN-FAIL（%d 条）' % len(kept)); rc = 1
    elif stale:
        print('❌ M279-LOOPSCAN-FAIL（豁免过期 %d 条）' % len(stale)); rc = 1
    else:
        print('M279-LOOPSCAN-OK')
    return rc


if __name__ == '__main__':
    sys.exit(main())
