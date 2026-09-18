#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_corpus.py — 生成 M142 语料（**单一事实源**：Go 真值程序与 PuXian 侧读同一份）

为什么用 hex 编码：语料要覆盖「原始控制字符 / 非法 UTF-8 / 只有空白 / 空输入 /
行尾空白」这些**无法用文本行安全表达**的字节串。每行 = 'x' + hex。
'#' 开头为注释。
"""
import json
import sys

CASES = []


def add(b):
    CASES.append(b if isinstance(b, bytes) else b.encode())


# ── 合法：结构 ──
add(b'{}')
add(b'[]')
add(b'[[]]')
add(b'[[],[]]')
add(b'{"a":1}')
add(b'{"a":1,"b":[1,2,{"c":null}]}')
add(b'{"a":[],"b":{}}')
add(b'{"":""}')
add(b'{"a":{"b":{"c":{"d":[[[]]]}}}}')
add(b'[{},[],{"k":[]}]')
add(b'{"nested":{"deep":{"deeper":[1,{"x":true},null,"s"]}}}')

# ── 合法：空白与幂等（Indent 输出的第二遍必须稳定）──
add(b'{"a": 1,\n  "b": [1,2]}')
add(b'{\n  "a": 1,\n  "b": [\n    1,\n    2\n  ]\n}')
add(b'{"a":1,"b":[1,2,{"c":null}]}\n')
add(b'\n\n\t {"a":1} \t\n')
add(b'{"a":1}   ')          # 尾部空白（Indent 保留 / Compact 丢弃）
add(b'{"a":1}]'[:-1])       # 同上去掉多余字符，保持合法
add(b'[1, 2, 3]')

# ── 合法：标量 ──
add(b'true')
add(b'false')
add(b'null')
add(b'123')
add(b'-0')
add(b'0')
add(b'1.5')
add(b'1.50')                # 原文保留（不重新格式化数字）
add(b'-0.0e+10')
add(b'1e0')
add(b'1e-7')
add(b'1e21')
add(b'12345678901234567890')
add(b'"abc"')
add(b'""')

# ── 合法：字符串内容（含转义与多字节，Indent 必须**原样**搬运）──
add(b'"a\\"b"')
add(b'"\\\\"')
add(b'"\\/\\b\\f\\n\\r\\t"')
add(b'"\\u4e2d"')
add(b'"\\ud83d\\ude00"')
add(b'{"\\u4e2d":"\\u503c"}')
add('{"中文":"值"}')
add(b'{"s":"<script>alert(1)</script>&x"}')
add(b'{"s":"\\u2028\\u2029"}')
add('{"s":"\u2028\u2029"}')          # 裸 U+2028/2029（非转义）—— Marshal 转义路径的靶子
add(b'{"s":"a\\tb"}')
add(b'["<", ">", "&"]')

# ── 非法：语法错误（文案 + 偏移必须逐字节一致）──
add(b'{')
add(b'[')
add(b'[1')
add(b'{"a"')
add(b'{"a":')
add(b'{"a":1')
add(b'[1,')
add(b'[1,]')
add(b'{"a":1,}')
add(b"{'a':1}")
add(b'5.1.2')
add(b'01')
add(b'-')
add(b'+1')
add(b'[1 2]')
add(b'"unterminated')
add(b'"\\x"')
add(b'"\\u12"')
add(b'"\\u12g4"')
add(b'{"a":1}}')
add(b'{"a":1} extra')
add(b'tru')
add(b'nul')
add(b'fals')
add(b'TrUe')
add(b'NaN')
add(b'Infinity')
add(b'"a\nb"')                       # 字符串内裸换行
add(b'"a\x01b"')                     # 字符串内裸控制字符
add(b'\xef\xbb\xbf{}')               # BOM
add(b'{"a" 1}')
add(b'[1,,2]')
add(b'{"a":1,"a"}')
add(b'   ')                          # 只有空白
add(b'')                             # 空输入

# ── 非法：错误字符为**非 ASCII 字节**（考验 quoteChar 的 Latin-1 语义）──
add(b'{\xe4}')                       # 0xE4 → '\u00e4'
add(b'[\x80]')                       # 0x80 → '\u0080'
add(b'{\xad:1}')                     # 0xAD（软连字符）→ '\u00ad'
add(b'[\xa1]')                       # 0xA1 可打印 → '¡'
add(b'{\xc3\xa9:1}')                 # 真 UTF-8 é（首字节 0xC3）→ '\u00c3'

# ── 深嵌套（合法 200 层 / 超深 10001 层触发 max depth）──
add(b'[' * 200 + b']' * 200)
add(b'[' * 10001 + b']' * 10001)

# ── 真实形态：token-cache 的 chat 请求体 ──
add(json.dumps({
    "model": "gpt-4o-mini",
    "messages": [
        {"role": "system", "content": "你是助手"},
        {"role": "user", "content": "帮我查一下北京今天天气\n换行也要保留"},
    ],
    "stream": False,
    "temperature": 0.7,
    "extra": {"a": [1, 2, {"b": None}], "t": True},
}, ensure_ascii=False).encode())


def main():
    out = ["# gen_corpus.py 生成 —— 每行 'x' + hex（非 ASCII 与空行都安全）",
           "# 行序 = 两侧索引序，禁止手工重排"]
    for c in CASES:
        out.append("x" + c.hex())
    data = "\n".join(out) + "\n"
    path = sys.argv[1] if len(sys.argv) > 1 else "corpus.txt"
    with open(path, "w", encoding="utf-8") as f:
        f.write(data)
    print("cases=%d bytes=%d -> %s" % (len(CASES), len(data), path))


if __name__ == "__main__":
    main()
