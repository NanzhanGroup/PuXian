#!/usr/bin/env python3
# ============================================================
# M198 判据 [S9]：native **元数上界** 的静态单一真相（缺陷 234）
# ------------------------------------------------------------
# 为什么需要：
#   M190（缺陷 213-m）量过「三轨 **rc 分叉**」；M197（缺陷 232/233）量过「**0 参** ⇄ arity 文案」。
#   两者都看不见**三轨一致**的宽容：`quic_close(1, 2)` 三个轨都**静默忽略**多余实参。
#   本轮普查（`runtime/*.c` 全量）实测：**81 个注册 native 没有显式上界**，其中
#   62 个是「固定元数却只查下界」（`nargs < N`，多余实参静默丢弃）+ 19 个「0/0-1 参」。
#   唯一仍宽容的必须是**真变长**语义（表内每条给理由）。
# 判据（硬，无棘轮）：
#   ① 注册表里的**每个** native 必须满足二者之一：
#        · 显式声明上界 —— 函数体里出现 `nargs != N` 或 `nargs > N`（N 为元数边界）；
#        · 在 `VARIADIC` 表内（真变长/可选尾参），**且表内每条名字都真的注册过**（防表与代码漂移）；
#   ② 元数检查必须用**形参名本身**（`nargs` / `n`）—— 排除局部变量同名造成的假阳性
#      （本脚本 v1 的自身缺陷：把局部 `int n` 的 `n < 0` 当成元数检查；v2 修正后
#        「无上界」由 69 变 81）。
#   ③ 打印**豁免表**与**声明分布**，供人复核（数量做**下限**断言，不写等式 —— 新增 native
#      时数量自然增长，只有「未声明 = 0」是恒等判据）。
# 用法：python3 sweep_upper.py [--root <仓库根>]     退出码 0=绿 1=红
# ============================================================
import os, re, sys

ROOT = "/data/code/puxian"
for i, a in enumerate(sys.argv):
    if a == "--root" and i + 1 < len(sys.argv):
        ROOT = sys.argv[i + 1]
RD = os.path.join(ROOT, "runtime")

# ── 真变长豁免表（**每条必须给理由**；名字必须真的在注册表里，否则判红） ──
VARIADIC = {
    "print": "变长打印（任意个数实参）",
    "print_err": "变长打印到 stderr",
    "min": "变长：min(a,b,…) 或 min(容器)",
    "max": "变长：max(a,b,…) 或 max(容器)",
    "range": "1-3 参（`nargs == k` 是**行为分叉**，不是元数边界）",
    "bytes_concat": "变长：拼接任意个 bytes",
    "set_timeout": "≥2 参 + 尾部实参**透传给回调**（fn, ms[, …args]）",
    "set_interval": "同上（fn, ms[, …args]）",
    "quic_connect": "3 或 4 参（第 4 参 = 可选 session hex；已显式 `nargs != 3 && nargs != 4` 校验）",
    "quic_connect_0rtt": "固定 4 参（已显式校验）",
    "input": "0-1 参（已显式 `nargs > 1` 校验）",
    "panic": "0-1 参（已显式 `nargs > 1` 校验）",
}


def strip_comments(txt):
    out = []
    for ln in txt.split("\n"):
        st = ln.lstrip()
        out.append("" if (st.startswith("//") or st.startswith("*") or st.startswith("/*")) else ln)
    return "\n".join(out)


def main():
    files = {}
    for f in sorted(os.listdir(RD)):
        if f.endswith(".c"):
            files[f] = strip_comments(open(os.path.join(RD, f), encoding="utf-8", errors="replace").read())

    # 注册表：语言层名字 → C 函数
    reg = {}
    reg_re = re.compile(r'px_set_global\(\s*"([^"]+)"\s*,\s*px_native\(\s*"([^"]+)"\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)')
    for f, txt in files.items():
        for m in reg_re.finditer(txt):
            reg.setdefault(m.group(1), (m.group(3), f))

    # 函数体
    body_re = re.compile(r'^\s*(?:static\s+)?(?:inline\s+)?LXValue\s+([A-Za-z_][A-Za-z0-9_]*)\s*'
                         r'\(\s*LXValue\s*\*\s*(?:args|a)\s*,\s*int\s+(nargs|n)\s*,[^)]*\)\s*\{', re.M)
    bodies = {}
    for f, txt in files.items():
        for m in body_re.finditer(txt):
            start = m.end() - 1
            depth, i = 0, start
            while i < len(txt):
                if txt[i] == "{":
                    depth += 1
                elif txt[i] == "}":
                    depth -= 1
                    if depth == 0:
                        break
                i += 1
            bodies.setdefault(m.group(1), (txt[start:i + 1], f, m.group(2)))

    declared, exempt, undeclared, nobody, drift = [], [], [], [], []
    for name, (cfn, fsrc) in sorted(reg.items()):
        b = bodies.get(cfn)
        if not b:
            nobody.append((name, cfn, fsrc))
            continue
        body, _, ac = b
        conds = [(m.group(1), int(m.group(2)))
                 for m in re.finditer(r'\b%s\s*(!=|==|<|>|<=|>=)\s*(\d+)' % re.escape(ac), body)]
        if any(op in ("!=", ">") for op, _ in conds):
            declared.append(name)
        elif name in VARIADIC:
            exempt.append(name)
        else:
            undeclared.append((name, cfn, fsrc, conds))

    for name in VARIADIC:
        if name not in reg:
            drift.append(name)

    print("── [S9] native 元数上界（静态） ──")
    print("注册 native 总数            : %d" % len(reg))
    print("显式声明上界（!= N / > N）  : %d" % len(declared))
    print("真变长豁免（表内，给理由）   : %d" % len(exempt))
    print("**未声明（必须为 0）**      : %d" % len(undeclared))
    print("解析不到函数体              : %d" % len(nobody))
    print("豁免表漂移（表内有、代码无） : %d" % len(drift))
    print("")
    print("豁免表（%d 条，每条附理由）：" % len(VARIADIC))
    for k in sorted(VARIADIC):
        print("   %-24s %s" % (k, VARIADIC[k]))

    bad = 0
    if drift:
        print("\n❌ 豁免表漂移：%s（表里写了但注册表没有）" % ", ".join(drift))
        bad += 1
    if nobody:
        print("\n❌ 注册了但找不到函数体（注册表与实现漂移）：")
        for n, c, f in nobody:
            print("   %s → %s (%s)" % (n, c, f))
        bad += 1
    if undeclared:
        print("\n❌ 没有元数上界且不在豁免表（多余实参会被**静默忽略**）：")
        for n, c, f, conds in undeclared:
            print("   %-30s %-22s %-20s conds=%s" % (n, f, c, conds))
        bad += 1
    if len(declared) < 250:
        print("\n❌ 声明上界的 native 只有 %d 个（< 250）—— 扫描器可能失效" % len(declared))
        bad += 1
    print("")
    print("── [S9] 结论：%s ──" % ("不一致 0" if bad == 0 else "不一致 %d" % bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
