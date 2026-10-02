#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
里程碑 ↔ 缺陷编号 一致性守卫（M241）
=================================================
背景（M241 实测）
-----------------
M240 的代码注释里 4 处写 `M240（缺陷 341）`，而 M240 实际编号是 **395–399**
（341 是 M227「同名两门对拍」的编号，与本处毫无关系）。
⇒ 后果：读者按「缺陷 341」去查，落到一个**完全无关**的里程碑上 —— 与 M185
  「错误信息指不到根因」同族，只不过指向错误的对象是**注释**而不是报错。
本轮同族还照出第二处：`runtime/vm.c` 把「静态审计器漏算分配点」记成 `缺陷 269`，
而那是 `缺陷 270`（269 是 `g_tmp_root` 出口那条，毫不相干）。

判据（唯一）
------------
源码里每一条 `M<m>（…缺陷 <n>…）` 引用，`n` 必须落在 **最近的那个里程碑标记**
所声明的编号集合内（集合从 `CHANGELOG.md` 的**自带标题行**派生）。

- 「最近的那个里程碑标记」= 在 `缺陷 <n>` **之前**最靠右的 `M<...>`。
  例：`M240（M235s1 缺陷 354 的镜像面）` ⇒ 归 M235s1（合法交叉引用）。
- 编号可带字母后缀（`213-m` / `243-b`）⇒ 取主号参与判定。
- 声明集合解析自标题行「缺陷」之后的**一整段**：支持
  `缺陷 395–399` / `缺陷 268 / 269 / 270` / `缺陷 234 + 235 + 229` / `缺陷 259–264 · 266`。
- **只认「自带标题」**（`^#+\\s*M<num>`）—— 交叉引用小标题（`### 二 派生索引（M235s1 缺陷 355 的教训）`）
  不构成声明。
- **声明集合解析不出来（标题行没写编号）⇒ 该里程碑的引用整批记为 SKIP 并响亮计数**
  （不许静默放过 —— M212 缺陷 288 的纪律）。
- 里程碑标记可带后缀（`M235s1` / `M89-S3-A5`）⇒ 优先用带后缀的名字查表，查不到退化为基号。
- 外来命名空间（`qg-issue 87 缺陷 7` / `第三方缺陷 018…030`）**紧贴**「缺陷」时跳过 ——
  那是对方问题单的编号，不是本仓的。

判据自身踩过的坑（都留成了 `--self-test` 夹具）
-----------------------------------------------
1. 标题行只认 `缺陷 <n>` 紧跟的单号 ⇒ M208 只解析成 {268} ⇒ **121 条假阳**（判据过窄会瞎报）。
2. 声明段截断用「半角/全角右括号取最小」⇒ `len(bytes)` 里的 `)` 把段砍半 ⇒ M175 的 `缺陷 14`
   判成未声明 = **判据自己造的假阳**。改「按最近前置左括号配对」。
3. 「本行出现过 qg-issue」就整行毙掉 ⇒ 把 M155 这类**自己的**声明也毙了（SKIP 63→94）⇒
   改「外来标记必须紧贴缺陷」。
4. 交叉引用小标题被当成声明 ⇒ M235s1 声明集错认成 {355}。改「只认自带标题」。

退出码
------
0 = 无违例；1 = 有违例；2 = 用法/输入错误；3 = **判据自身失效**（CHANGELOG 一条声明都没解析出来）
"""
import os
import re
import sys
import json
import glob
import argparse

# 里程碑标记：M235 / M235s1 / M89-S3-A5
RE_MILE = re.compile(r'\bM(\d+)((?:[-_a-zA-Z]+\d*)*)\b')
# **自带标题**（本里程碑自己的标题行）—— 只有这种行才算「声明」
RE_OWN_HEAD = re.compile(r'^#+\s*M(\d+)((?:[-_a-zA-Z]+\d*)*)')
# 缺陷号：缺陷 399 / 缺陷 213-m
RE_DEF = re.compile(r'缺陷\s*([0-9]+)([a-zA-Z]?)')
# 裸数字（不含与字母/下划线相邻者 —— 排除 M240_PLAN / qg-issue87）
RE_BARE_NUM = re.compile(r'(?<![A-Za-z0-9_])[0-9]+(?![0-9])')
# 区间：15–55 / 259–264（只隔连字符或波浪号）
RE_RANGE = re.compile(r'(?<![0-9])([0-9]+)\s*[–—~]\s*([0-9]+)(?![0-9])')
# **外来命名空间**：必须紧贴「缺陷」——`qg-issue 87 缺陷 7` / `第三方缺陷 018…030`
RE_FOREIGN = re.compile(r'(?:第三方|qg[-_ ]?issue\s*\d+|PX-DEF[-\s]?\d+)\s*缺陷')

# 默认扫描面（源码里的注释引用）
DEFAULT_GLOBS = ['runtime/*.c', 'runtime/*.h', 'selfhost/*.px']
# 外来命名空间检查的回看窗口（字符）
FOREIGN_LOOKBACK = 24


def cut_group(text, start):
    """从 text[start:] 找第一个**带编号的**「缺陷 <n>」，返回它所属括号组的切片 + 绝对下标。

    ⚠️ 必须要求「缺陷」后面**紧跟数字** —— 标题里常见「晨曦缺陷 A/B 收口」这种**不带编号**的
    用法（M240 的标题就是），首版逮到它当锚点 ⇒ 声明段吞掉整行 ⇒ M240 的声明集多出
    「第 117 轮」的 **117**（假声明）。
    括号组 = 与**最近的前置左括号**配对的那一对（找不到左括号 ⇒ 到行尾）。
    """
    m = RE_DEF.search(text, start)
    if not m:
        return None, -1
    d = m.start()
    lo, lo_ch = -1, None
    for ch in ('（', '('):
        p = text.rfind(ch, 0, d)
        if p > lo:
            lo, lo_ch = p, ch
    close = '）' if lo_ch == '（' else (')' if lo_ch == '(' else None)
    if close is None:
        end = len(text)
    else:
        p = text.find(close, d)
        end = p if p >= 0 else len(text)
    return text[d:end], d


def decl_of_seg(seg):
    """声明的编号集合：区间展开 + 裸数字（先剔除「第 N 轮」这类**轮号**，免得被当成缺陷号）"""
    seg = re.sub(r'第\s*[0-9]+\s*轮', ' ', seg)
    s = set()
    for a, b in RE_RANGE.findall(seg):
        a, b = int(a), int(b)
        if 0 < a <= b and b - a < 500:
            s.update(range(a, b + 1))
    s.update(int(x) for x in RE_BARE_NUM.findall(seg))
    return s


def parse_changelog(path):
    """→ ({milestone_key: set(int)}, {无声明的里程碑})"""
    decl, no_decl = {}, set()
    with open(path, encoding='utf-8', errors='replace') as f:
        for line in f:
            if not line.startswith('#'):
                continue
            om = RE_OWN_HEAD.match(line)          # 只认自带标题
            if not om:
                continue
            key = 'M' + om.group(1) + (om.group(2) or '')
            seg, d = cut_group(line, 0)
            if seg is None:
                no_decl.add(key)
                continue
            if RE_FOREIGN.search(line[max(0, d - FOREIGN_LOOKBACK):d + 2]):
                # `第三方缺陷 018…030` ⇒ 那串号是对方的命名空间，本行不声明自己的编号
                no_decl.add(key)
                continue
            s = decl_of_seg(seg)
            if s:
                decl[key] = s
                decl.setdefault('M' + om.group(1), set()).update(s)
            else:
                no_decl.add(key)
    return decl, no_decl


def scan_file(path):
    """→ [(lineno, milestone_key, defect_int, raw)]"""
    out = []
    with open(path, encoding='utf-8', errors='replace') as f:
        for i, line in enumerate(f, 1):
            for pm in RE_MILE.finditer(line):
                seg, d = cut_group(line, pm.end())
                if seg is None:
                    continue
                dm = RE_DEF.search(seg)
                if not dm:
                    continue
                if RE_FOREIGN.search(line[max(0, d - FOREIGN_LOOKBACK):d + 2]):
                    continue                      # `qg-issue 87 缺陷 7` ⇒ 对方编号
                out.append((i, pm.group(0), int(dm.group(1)), dm.group(0)))
    return out


def main():
    ap = argparse.ArgumentParser(description='里程碑 ↔ 缺陷编号 一致性守卫')
    ap.add_argument('--root', default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                    help='仓库根（默认由脚本位置推断）')
    ap.add_argument('--changelog', default=None, help='CHANGELOG 路径（默认 <root>/CHANGELOG.md）')
    ap.add_argument('--json', action='store_true', help='输出 JSON')
    ap.add_argument('--quiet', action='store_true', help='只输出结论行')
    ap.add_argument('--self-test', action='store_true', help='对内置 fixture 做自证')
    args = ap.parse_args()

    if args.self_test:
        return selftest(os.path.abspath(args.root))

    root = os.path.abspath(args.root)
    cl = args.changelog or os.path.join(root, 'CHANGELOG.md')
    if not os.path.exists(cl):
        print('❌ 找不到 CHANGELOG: %s' % cl, file=sys.stderr)
        return 2

    decl, no_decl = parse_changelog(cl)
    if not decl:
        print('❌ 判据自身失效：CHANGELOG 里一条编号声明都没解析出来', file=sys.stderr)
        return 3

    files = []
    for g in DEFAULT_GLOBS:
        files.extend(sorted(glob.glob(os.path.join(root, g))))

    viol, checked, skipped, unknown = [], 0, 0, {}
    seen_files = 0
    for fp in files:
        if not os.path.isfile(fp):
            continue
        seen_files += 1
        for (ln, mile, num, raw) in scan_file(fp):
            s = decl.get(mile)
            if s is None:
                base = 'M' + RE_MILE.match(mile).group(1)
                s = decl.get(base)
            if s is None:
                skipped += 1
                unknown[mile] = unknown.get(mile, 0) + 1
                continue
            checked += 1
            if num not in s:
                viol.append({'file': os.path.relpath(fp, root), 'line': ln,
                             'milestone': mile, 'defect': num, 'raw': raw,
                             'declared': sorted(s)[:6] + (['…'] if len(s) > 6 else []),
                             'declared_n': len(s)})

    ok = not viol
    if args.json:
        print(json.dumps({'ok': ok, 'checked': checked, 'skipped': skipped,
                          'files': seen_files, 'milestones_with_decl': len(decl),
                          'violations': viol, 'skipped_milestones': unknown},
                         ensure_ascii=False, indent=2))
    else:
        if not args.quiet:
            print('扫描 %d 个源码文件 · 里程碑声明表 %d 条' % (seen_files, len(decl)))
            print('引用检查：判定 %d 条 · 跳过 %d 条（里程碑无编号声明）' % (checked, skipped))
            if unknown:
                print('  跳过的里程碑标记：%s'
                      % ', '.join('%s×%d' % (k, v) for k, v in sorted(unknown.items())))
        if ok:
            print('✅ 里程碑 ↔ 缺陷编号 一致（判定 %d 条 · 跳过 %d 条）' % (checked, skipped))
        else:
            print('❌ 里程碑 ↔ 缺陷编号 不一致：%d 条' % len(viol))
            for v in viol:
                print('  %s:%d  %s（%s） ⇒ %d 不在该里程碑声明的编号里（声明 %d 个: %s）'
                      % (v['file'], v['line'], v['milestone'], v['raw'], v['defect'],
                         v['declared_n'], v['declared'][:6]))
    return 0 if ok else 1


def selftest(root):
    """自证：内置 fixture 检查判据本身有牙（不依赖仓库内容）"""
    import tempfile
    w = tempfile.mkdtemp(prefix='m241refs_')
    os.makedirs(os.path.join(w, 'runtime'), exist_ok=True)
    cl = os.path.join(w, 'CHANGELOG.md')
    src = os.path.join(w, 'runtime', 'a.c')
    with open(cl, 'w', encoding='utf-8') as f:
        f.write('## M240 · 主题（缺陷 395–399）（第 117 轮）\n')
        f.write('## M235s1 · 补（缺陷 354 / 355）\n')
        f.write('## M999 · 没有编号的标题\n')
        # 交叉引用小标题 —— 首版把它当成了 M777 的「声明」（实际 M777 没有自带标题）
        f.write('### 二 派生索引三处同步（M777 缺陷 999 的教训）\n')
        # 「半角 ) 在声明段里」的回归位（M175 形状）
        f.write('## M175 · 两小项（第 57 轮 · 缺陷 153 `len(bytes)` + 缺陷 14 `os_popen`）\n')
        # 外来命名空间（M189 形状）
        f.write('## M189 · 边界族 + 第三方缺陷 018…030 全量判定（第 67 轮）\n')

    def run(content):
        with open(src, 'w', encoding='utf-8') as f:
            f.write(content)
        d, _ = parse_changelog(cl)
        out = []
        for (ln, mile, num, raw) in scan_file(src):
            s = d.get(mile)
            if s is None:
                s = d.get('M' + RE_MILE.match(mile).group(1))
            out.append((mile, num, 'SKIP' if s is None else ('OK' if num in s else 'VIOL')))
        return out

    cases = [
        ('// M240（缺陷 399）\n', [('M240', 399, 'OK')], '命中声明'),
        ('// M240（缺陷 341）\n', [('M240', 341, 'VIOL')], '★ 越界编号 —— 本轮修的正是这形状'),
        ('// M240（M235s1 缺陷 354 的镜像面）\n', [('M240', 354, 'VIOL'), ('M235s1', 354, 'OK')],
         '交叉引用：最近标记胜出（M235s1 有自带标题 ⇒ OK）'),
        ('// M777（缺陷 999）\n', [('M777', 999, 'SKIP')],
         '★ 交叉引用小标题不构成声明（M235s1 真仓里无自带标题的形状）'),
        ('// M240（缺陷 B 之三）\n', [], '无编号 ⇒ 不判'),
        ('// M999（缺陷 1）\n', [('M999', 1, 'SKIP')], '里程碑无声明 ⇒ SKIP 而非放过'),
        ('// M240（缺陷 395）与 M240（缺陷 400）\n', [('M240', 395, 'OK'), ('M240', 400, 'VIOL')],
         '一行两处，逐个判'),
        ('// M129（qg-issue 87 缺陷 7）\n', [], '★ 外来命名空间（对方问题单编号）⇒ 不参与判定'),
        ('// M175（缺陷 14）：x\n// M175（缺陷 153 `len(bytes)`）\n',
         [('M175', 14, 'OK'), ('M175', 153, 'OK')],
         '★ 半角 `)` 不得把声明段砍半（M175 形状）'),
        ('// M189（缺陷 211）\n', [('M189', 211, 'SKIP')],
         '★ 第三方缺陷 0NN…0NN 不得被当成本里程碑的编号'),
    ]
    bad = 0
    for content, want, name in cases:
        got = run(content)
        if got != want:
            bad += 1
            print('  ❌ fixture %-44s 期望 %s 实得 %s' % (name, want, got))
        else:
            print('  ✅ fixture %-44s %s' % (name, got))

    # 判据自伤：CHANGELOG 一条声明都没有 ⇒ 必须 exit 3
    with open(cl, 'w', encoding='utf-8') as f:
        f.write('## M240 · 无编号标题\n')
    d, _ = parse_changelog(cl)
    if d:
        bad += 1
        print('  ❌ 自伤判据：无声明时 decl 应为空，实得 %s' % d)
    else:
        print('  ✅ 自伤判据：无声明 ⇒ decl 空 ⇒ 主流程 exit 3')

    print('SELFTEST-%s  (%d 例 · %d 失败)' % ('OK' if bad == 0 else 'FAIL', len(cases) + 1, bad))
    return 0 if bad == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
