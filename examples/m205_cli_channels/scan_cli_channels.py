#!/usr/bin/env python3
# ============================================================
# M205 [S12]：CLI 工具的**诊断通道**静态判据（tools/ 面 · 缺陷 186 收尾）
# ------------------------------------------------------------
# 口径（docs/ERROR_CODES.md §7）：
#   · **诊断**（错误 / 警告 / 参数错提示 / 参数错时的用法）⇒ **stderr**
#   · **产品**（成功路径上的结果、结构化输出、`--help`/`--version` 文本）⇒ **stdout**
#   · 失败路径 **rc≠0**
# 判据：
#   A 失败路径 print 站点 = 0（按"诊断词前缀"识别，不看窗口 ⇒ 不误伤产品行）
#   B 每个 CLI 的 usage 必须带 `to_err` 形参且体内真实分流
#   C stderr 出口闭集（print_err / px_err / lsp_err / mcp_err / write(2）—— 附**锚点自证**（规模下限）
#   D shell 侧（tools/px、tools/pxpkg）的 `*)` 未知命令分支必须 `>&2` + 非零退出
#   E **反向判据**：产品行仍在 stdout（防"一刀切全改 stderr"把工具产品也搬到 stderr）
# 用法：python3 scan_cli_channels.py <repo-root>   → 打印判据结果；退出 0=全绿 1=有红
# ============================================================
import os
import re
import sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else '.'
os.chdir(ROOT)

CLI_FILES = ['tools/pxfmt.px', 'tools/pxtest.px', 'tools/pxbench.px', 'tools/pxdoc.px',
             'tools/pxlint.px', 'tools/pxcheck.px', 'tools/pxlsp.px', 'tools/pxmcp.px',
             'tools/pxpkg.px', 'tools/routegen.px']
USAGE_FUNCS = ['usage', 'pt_usage', 'pb_usage', 'pd_usage']   # 8 个 CLI 共用这 4 个名字
# 诊断词前缀：出现在 print( 字面量开头 ⇒ 这是"给用户看的诊断"，必须在 stderr
DIAG_PREFIX = re.compile(r'^\s*print\(\s*"(错误|警告|失败|无法|非法|未知|用法:|\[错误\])')
# 产品行（成功路径的"工具产品"）—— 必须留在 stdout
# 产品行（成功路径的"工具产品"）—— 必须留在 stdout。按**行级**匹配（不要求紧跟引号位置）
PRODUCT_PAT = re.compile(r'^\s*print\(.*(结果:|安装 |包名:|已写入|已格式化|格式正确|格式需要调整|校验通过|基准:|错误, |警告|第 \" )')

fails = []
notes = []


def read(p):
    return open(p, encoding='utf-8').read()


# ---------- A：失败路径 print 站点 = 0 ----------
a_hits = []
for f in CLI_FILES:
    if not os.path.exists(f):
        fails.append(f'A 缺文件 {f}')
        continue
    for i, ln in enumerate(read(f).split('\n'), 1):
        if DIAG_PREFIX.match(ln):
            a_hits.append(f'{f}:{i}: {ln.strip()[:88]}')
if a_hits:
    fails.append(f'A 失败路径仍走 stdout 的 print 站点 {len(a_hits)} 个：\n      ' + '\n      '.join(a_hits[:6]))
else:
    notes.append('A 失败路径 print 站点 = 0')

# ---------- B：usage 带 to_err 且真实分流 ----------
b_bad = []
b_ok = []
for f in CLI_FILES:
    src = read(f)
    for m in re.finditer(r'^def (\w*usage)\(([^)]*)\):', src, re.M):
        name, params = m.group(1), m.group(2)
        body = src[m.end(): m.end() + 1400]
        # 取到下一个 top-level def 之前
        nxt = re.search(r'^def ', body, re.M)
        if nxt:
            body = body[:nxt.start()]
        if 'to_err' not in params or 'to_err' not in body:
            b_bad.append(f'{f}: def {name}({params}) 未按 to_err 分流')
        else:
            b_ok.append(f'{f}:{name}')
if b_bad:
    fails.append('B usage 未分流：\n      ' + '\n      '.join(b_bad))
elif len(b_ok) < 8:
    fails.append(f'B 只找到 {len(b_ok)} 个带 to_err 的 usage（期望 ≥8）')
else:
    notes.append(f'B usage 分流 {len(b_ok)} 个（≥8）')

# ---------- C：stderr 出口闭集 + 锚点自证 ----------
KINDS = {'print_err': 0, 'px_err': 0, 'lsp_err': 0, 'mcp_err': 0, 'write(2': 0}
for f in CLI_FILES:
    src = read(f)
    for k in KINDS:
        KINDS[k] += len(re.findall(re.escape(k) + r'\s*\(', src))
c_total = sum(KINDS.values())
if c_total < 60:
    fails.append(f'C stderr 出口站点只有 {c_total}（下限 60 · 锚点自证失败 ⇒ 扫描器可能失效）')
else:
    notes.append('C stderr 出口 ' + ' · '.join(f'{k}={v}' for k, v in KINDS.items()) + f'（合计 {c_total}）')

# ---------- D：shell 侧未知命令分支 ----------
d_bad = []
for sh, marker in [('tools/px', '未知子命令: $cmd'), ('tools/pxpkg', 'pxpkg: 未知命令')]:
    if not os.path.exists(sh):
        d_bad.append(f'{sh} 不存在')
        continue
    src = read(sh)
    if marker not in src:
        d_bad.append(f'{sh} 缺「{marker}」分支')
        continue
    seg = src[src.index(marker): src.index(marker) + 120]
    if '>&2' not in seg or not re.search(r'exit\s+[1-9]', seg):
        d_bad.append(f'{sh} 的「{marker}」未走 stderr+非零退出：{seg.splitlines()[0].strip()[:70]}')
if d_bad:
    fails.append('D shell 侧：\n      ' + '\n      '.join(d_bad))
else:
    notes.append('D shell 侧 px / pxpkg 的未知命令分支 = stderr + 非零')

# ---------- E：反向判据（产品仍在 stdout） ----------
e_ok = 0
for f in CLI_FILES:
    src = read(f)
    e_ok += sum(1 for ln in src.split('\n') if PRODUCT_PAT.match(ln))
    e_ok += len(re.findall(r'^\s*print\(md\)', src, re.M))          # pxdoc 正文
    e_ok += len(re.findall(r'^\s*print\(json_stringify\(', src, re.M))  # pxcheck JSON 通道
if e_ok < 12:
    fails.append(f'E 反向判据：产品行只剩 {e_ok} 处（下限 12 —— 可能被"一刀切"搬去了 stderr）')
else:
    notes.append(f'E 产品行仍在 stdout：{e_ok} 处（≥12）')

# ---------- 输出 ----------
print('[S12] CLI 诊断通道（tools/ 面）静态判据')
for n in notes:
    print(f'  ✅ {n}')
for f in fails:
    print(f'  ❌ {f}')
print(f'  ── 判据 A/B/C/D/E：{"全绿" if not fails else f"失败 {len(fails)} 项"}')
sys.exit(0 if not fails else 1)
