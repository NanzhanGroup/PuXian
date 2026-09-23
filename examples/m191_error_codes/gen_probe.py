#!/usr/bin/env python3
# 生成 M191 动态探针用例（每个用例：先 print("before") 证运行期，再触发目标错误）
import os
D = os.path.join(os.path.dirname(os.path.abspath(__file__)), "probe")
os.makedirs(D, exist_ok=True)
CASES = {
    "d1_slice_step0": (['var l = [1, 2, 3]', 'print("before")', 'print(l[0:2:0])'], "R1006", "切片步长不能为 0"),
    "d2_slice_type": (['var x = 1.5', 'print("before")', 'print(x[0:1])'], "R1002", "此类型不支持切片: float"),
    "d3_len_type": (['def f():', '    return 0', 'print("before")', 'print(len(f))'], "R1002", "len 不支持类型 function"),
    "d4_str_upper_arity": (['var s = "ab"', 'print("before")', 's.upper(9)'], "R1005", "方法 upper 不接受参数"),
    "d5_assert": (['print("before")', 'assert(false)'], "R2001", "断言失败"),
    "d6_assert_msg": (['print("before")', 'assert(false, "boom")'], "R2001", "断言失败: boom"),
    "d7_panic": (['print("before")', 'panic("boom")'], "R2001", "boom"),
    "d8_range_step0": (['print("before")', 'print(range(0, 5, 0))'], "R1006", "range step 不能为 0"),
    "d9_i2b_endian": (['print("before")', 'print(int_to_bytes(1, 2, "middle"))'], "R1002", "int_to_bytes 的 endian 需为 big/little"),
    "d10_join_arity": (['print("before")', 'print(join("a"))'], "R1002", "join 需要 2 个参数（分隔符, 列表）"),
    "d11_split_type": (['print("before")', 'print(split(1, ","))'], "R1002", "split 参数需要 string"),
}
for k, (src, code, body) in CASES.items():
    open(os.path.join(D, k + ".px"), "w", encoding="utf-8").write("\n".join(src) + "\n")
with open(os.path.join(D, "EXPECT.tsv"), "w", encoding="utf-8") as f:
    for k, (_, code, body) in CASES.items():
        f.write("%s\t%s\t%s\n" % (k, code, body))
print("写出 %d 个用例 + EXPECT.tsv -> %s" % (len(CASES), D))
