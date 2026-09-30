#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M233 门 · 探针与期望表生成器（「文本语义接口 × 逐类型实参」全量对拍）

为什么是这个面
--------------
M195 的 `docs/ERROR_CODES.md` §6.5 立过一条**语义豁免**：
「参数本身就是一段**文本**」的接口（哈希 / 编码 / 正则 / 字符处理）对任意实参**宽容**，
语义是取「其 `str()` 形态」。M196/M199 把「守卫」这一侧写成了判据，
**但『其 str() 形态』这一侧从来没有被清单级度量过** —— 这一侧恰恰是
`val_cstr` / `bdata` / `blen` 三处兜底渲染器的职责。

缺陷 348 / 349 / 350（本轮立论）
--------------------------------
三处兜底都写 `snprintf("%s", fmt_num(v))`，而 `fmt_num` **只对 `int`/`float` 正确**
（它读 `v.as.i` / `v.as.f`）：

  · `bool`：`as.f` 是**位模式重解释** —— `true` 的 `as.b=1` 读成 double 得 `5e-324`
    ⇒ `sha256(true)` 算的是字符串 `"5e-324"`（**静默错值**）；
  · 容器 / `struct` / `enum` / `result` / `function` / `native`：`as.f` 读的是
    **`as.obj` 指针位** ⇒ ① **非确定**（同一二进制同一输入连跑 5 次得 5 个不同结果）
    ② **把堆地址位当文本吐出去**（`base64_encode([1])` 的 base64 里就是 ASLR 位）；
  · `g_tmp_ring` 每槽只有 64 字节 ⇒ 长渲染**静默截断**（缺陷 350）。

期望表的来历（如实登记）
------------------------
`MODEL.tsv` 的「响亮性」由**语言语义**写定（文本语义接口对任意值**不报错** ⇒ 一律 `VAL`），
**不是**从实测抄出来的；`verify.sh` 第 [4] 层做**双向**核对。

**独立真值**（不依赖三轨）：每个用例同时求 `f(x)` 与 `f(str(x))` ——
对**非 `bytes`** 实参二者必须逐字节相等（`bytes` 实参走对象自身缓冲，
语义是「原始字节」而不是「文本形态」，故单列，见覆盖边界）。

用法
----
  gen_probes.py --root R --gen        生成 cases.tsv + drv.px + MODEL.tsv
  gen_probes.py --root R --audit      源码派生：兜底渲染器是否已统一
"""
import argparse, os, re, sys

# 文本语义（§6.5 豁免表）里的一元接口。**源码派生**核对见 --audit。
FUNCS = ["sha256", "md5", "base64_encode", "ord", "bytes_to_hex"]

# 实参：类型 × 变体（值必须能在三轨里都构造出来）
ARGS = [
    ("int_0", "0"), ("int_7", "7"), ("int_neg", "0 - 7"),
    ("float_0", "0.0"), ("float_15", "1.5"), ("float_neg", "0.0 - 1.5"),
    ("str_empty", '""'), ("str_ab", '"ab"'), ("str_cjk", '"中文"'),
    ("bytes_ab", 'bytes("ab")'), ("bytes_empty", 'bytes("")'),
    ("bool_t", "true"), ("bool_f", "false"),
    ("null_v", "null"),
    ("list_1", "[1]"), ("list_empty", "[]"),
    ("dict_a1", '{"a": 1}'), ("dict_empty", "{}"),
    ("tuple_1", "(1,)"), ("tuple_empty", "()"),
    ("result_ok", "Ok(1)"), ("result_err", 'Err("e")'),
    ("enum_a", "E1.A"), ("struct_v", "S1(1)"), ("fn_v", "userfn"),
]

# 走「原始字节」而不是「文本形态」的实参（对齐判据必须豁免，理由写进覆盖边界）
RAW_BYTES = {"bytes_ab", "bytes_empty"}

HDR = '''struct S1:
    a: int

enum E1:
    A
    B

def userfn():
    return 1

'''


def tag_of(fn, arg):
    return "%s__%s" % (fn, arg)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--gen", action="store_true")
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--out", default=None)
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    d = a.out or os.path.join(root, "examples", "m233_text_cstr")

    if a.audit:
        return audit(root)

    cases = []
    for fn in FUNCS:
        for arg_tag, expr in ARGS:
            cases.append((tag_of(fn, arg_tag), fn, expr, arg_tag))
    if not a.gen:
        print("用例 %d（%d 函数 × %d 实参）" % (len(cases), len(FUNCS), len(ARGS)))
        return 0

    os.makedirs(d, exist_ok=True)
    src = [HDR]
    for i, (_tag, fn, expr, _at) in enumerate(cases):
        # 两列：f(x) 与 f(str(x)) —— 后者是**独立真值**（非 bytes 实参必须相等）
        src.append('def t%d():\n    print("%s|" + str(%s(%s)) + "|" + str(%s(str(%s))))\n'
                   % (i, _tag, fn, expr, fn, expr))
    src.append("def main():\n")
    src.append('    let c = env("M233_CASE")\n')
    src.append('    if c == null:\n        print("usage")\n        return\n')
    src.append("    let i = c\n")
    for i in range(len(cases)):
        kw = "if" if i == 0 else "elif"
        src.append('    %s i == "%d":\n        t%d()\n' % (kw, i, i))
    src.append('    else:\n        print("bad idx")\n')
    open(os.path.join(d, "drv.px"), "w").write("".join(src))

    with open(os.path.join(d, "cases.tsv"), "w") as f:
        f.write("idx\ttag\tfunc\targ\texpr\talign\n")
        for i, (tag, fn, expr, at) in enumerate(cases):
            al = "NO" if at in RAW_BYTES else "YES"
            f.write("%d\t%s\t%s\t%s\t%s\t%s\n" % (i, tag, fn, at, expr, al))

    with open(os.path.join(d, "MODEL.tsv"), "w") as f:
        f.write("tag\tkind\tbasis\n")
        for tag, _fn, _expr, _at in cases:
            f.write("%s\tVAL\ttext-semantics-tolerates-any\n" % tag)

    print("M233-GEN-OK cases=%d（%d 函数 × %d 实参；对齐判据 %d 例）"
          % (len(cases), len(FUNCS), len(ARGS), sum(1 for c in cases if c[3] not in RAW_BYTES)))
    return 0


def audit(root):
    """源码派生：三处兜底是否统一到 `px_cstr_any`（不许再有 `fmt_num` 兜底）"""
    src = open(os.path.join(root, "runtime", "runtime.c"), encoding="utf-8").read()
    bad = 0
    # ① 统一入口必须在位
    if "px_cstr_any" not in src:
        print("❌ 未找到 px_cstr_any（兜底渲染器未统一）"); bad += 1
    else:
        print("✅ px_cstr_any 在位")
    # ② 反向判据：不许再出现「snprintf(tmp, …, fmt_num(v))」这种**兜底**
    #    ⚠️ 先剔除 `px_cstr_any` 自身的**数字快路径**（那是**有意**的：int/float 走
    #       `fmt_num` 是正确的，且不分配）—— 不剔除会把正确实现判成违例（假红）。
    stripped = src
    m0 = re.search(r'static const char\* px_cstr_any\(LXValue v, int\* out_len\) \{(.*?)\n\}',
                   src, re.S)
    if m0:
        stripped = src.replace(m0.group(0), "")
    pat = re.compile(r'snprintf\(tmp,\s*PX_TMPSZ,\s*"%s",\s*fmt_num\(')
    hits = pat.findall(stripped)
    print("%s snprintf(tmp, …, fmt_num(…)) 兜底站点：%d 处（已剔除 px_cstr_any 的数字快路径）"
          % ("✅" if not hits else "❌", len(hits)))
    if hits:
        bad += 1
    # ③ 三个入口都必须走 px_cstr_any
    for fn, want in (("val_cstr", "px_cstr_any(v, NULL)"),
                     ("bdata", "px_cstr_any(v, NULL)"),
                     ("blen", "px_cstr_any")):
        m = re.search(r'static (?:const char\*|int) %s\(LXValue v\) \{(.*?)\n\}' % fn, src, re.S)
        if not m:
            print("❌ 未能定位 %s（锚点失效）" % fn); bad += 1; continue
        if want not in m.group(1):
            print("❌ %s 未走统一入口" % fn); bad += 1
        else:
            print("✅ %s → 统一入口" % fn)
    print("M233-AUDIT-OK" if bad == 0 else "M233-AUDIT-FAIL")
    return 0 if bad == 0 else 1


sys.exit(main())
