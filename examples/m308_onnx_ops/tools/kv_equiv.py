#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""kv_equiv.py —— M308 门 ③d：KV cache **增量解码的等价性**判据（M308-PLAN §二.S2 的硬判据）。

为什么单独一条腿（不能用 ③a/③b 代偿）：
  ③a（exec_ref）比对的是「**图 vs 手算真值**」、③b（ops_ref）比对的是「**图 vs 独立复算**」——
  两者都能证明「两个输出各自等于 1,2,3,4,5,6」，**但它们证明不了「两者互相等价」这件事本身**
  （传上去判断的是同一个手算表，属于同一条链）。而 §二.S2 要的恰恰是那条关系：
  「预分配 cache 上逐步 ScatterND 写入」⇄「一次性 Concat 全长」**逐值等价**。

判据（每条都能单独变红）：
  ① 两个输出**都在**（取不到判据 ≠ 判据满足）
  ② **形状相同**（[3,2]）且**逐值相等**（严格相等，逐位；KV cache 是搬运不是计算 ⇒ 不许有容差）
  ③ **写入真的发生了** —— `O_STEP` 必须与预分配 cache 的**原值不同**，
     且改动的**恰好是 SLOT 那一行**（其余行逐值不变）。
     ⚠️ 没有这一条，两边都返回原值也能「等价」过关（假绿的典型形状）。

用法：python3 tools/kv_equiv.py <C 侧 dump 目录>
退出码：0 全绿 / 1 判红 / 2 用法或输入缺失
"""
import json
import os
import struct
import sys

CACHE_ROWS = [[1.0, 2.0], [3.0, 4.0], [0.0, 0.0]]   # 与 gen_m308_models.py 的 CACHE 一致
SLOT = 2                                             # 本次写入的槽（0 基）
MODEL = "e16_kv_decode"


def f32_of(hexs):
    """C 侧 dump 是 fp32 的十六进制串 —— 按**实际位型**解回 float。"""
    return list(struct.unpack("<%df" % (len(hexs) // 8), bytes.fromhex(hexs)))


def main():
    if len(sys.argv) < 2:
        print("用法: python3 tools/kv_equiv.py <C 侧 dump 目录>")
        return 2
    dumps = sys.argv[1]
    dp = os.path.join(dumps, MODEL + ".json")
    if not os.path.exists(dp):
        print("  ❌ 取不到判据：缺 C 侧 dump %s" % dp)
        return 2
    d = json.load(open(dp))
    outs = d.get("outputs", {})
    bad = 0

    def ck(cond, label):
        nonlocal bad
        print("  %s %s" % ("✅" if cond else "❌", label))
        if not cond:
            bad += 1

    for nm in ("O_STEP", "O_ONCE"):
        if nm not in outs:
            print("  ❌ 取不到判据：dump 里没有输出 %s" % nm)
            return 2
    a, b = f32_of(outs["O_STEP"]["hex"]), f32_of(outs["O_ONCE"]["hex"])
    da, db = outs["O_STEP"]["dims"], outs["O_ONCE"]["dims"]

    ck(list(da) == [3, 2] and list(db) == [3, 2],
       "形状：O_STEP=%s · O_ONCE=%s（都应为 [3, 2]）" % (da, db))
    if len(a) != len(b):
        ck(False, "元素数 %d ≠ %d" % (len(a), len(b)))
        print("M308-KVEQ-FAIL(%d)" % bad)
        return 1
    ndiff = sum(1 for x, y in zip(a, b) if x != y)
    ck(ndiff == 0,
       "**逐值等价**：%d 点全部相等（增量路径 ⇄ 全长路径 · 严格相等，无容差）" % len(a)
       if ndiff == 0 else "逐值不等价：%d/%d 点不同（首处 %r vs %r）"
       % (ndiff, len(a), a[next(i for i in range(len(a)) if a[i] != b[i])],
          b[next(i for i in range(len(a)) if a[i] != b[i])]))

    # ③ 写入真的发生了：改动**恰好**是 SLOT 那一行，其余行逐值不变
    flat_cache = [v for row in CACHE_ROWS for v in row]
    changed = [i for i in range(len(a)) if a[i] != flat_cache[i]]
    want = [SLOT * 2, SLOT * 2 + 1]
    ck(sorted(changed) == want,
       "写入真的发生：改动下标 %s（应恰为 SLOT=%d 那一行 %s）" % (sorted(changed), SLOT, want))
    untouched = [i for i in range(len(a)) if i not in want]
    ck(all(a[i] == flat_cache[i] for i in untouched),
       "其余槽逐值未动（%d 个下标）" % len(untouched))
    ck(a[SLOT * 2] == 5.0 and a[SLOT * 2 + 1] == 6.0,
       "写入内容正确：第 %d 槽 = [5, 6]（= K_NEW）" % SLOT)

    print("M308-KVEQ: 增量 ⇄ 全长 %d 点逐值等价 · 写入落在第 %d 槽" % (len(a), SLOT))
    print("M308-KVEQ-%s" % ("OK" if bad == 0 else "FAIL(%d)" % bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
