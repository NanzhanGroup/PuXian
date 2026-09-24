#!/usr/bin/env python3
# ============================================================
# M202 门的**打桩器**：把「负控要改的那一处」集中在这里（一处一桩、失败即 rc=3）
# ------------------------------------------------------------
# 为什么单独一个文件（血泪）：负控的 old/new 文本若散在 shell 的 perl -e 里，
#   `\Q..\E` / 反斜杠 / 转义层数一多就会「打桩静默失败」——门却报「未判红」，
#   错误信息完全指不到真因（M202 实测踩过）。这里用 Python **单引号三引号字面量**
#   写死，找不到锚点就响亮退出 3（门据此判 FAIL 而不是假绿）。
# 用法：python3 stubs.py <A|B|C|D|E> [仓库根]
# ============================================================
import sys, os

ROOT = sys.argv[2] if len(sys.argv) > 2 else "/data/code/puxian"

RUNTIME = os.path.join(ROOT, "runtime/runtime.c")
IBUILTIN = os.path.join(ROOT, "selfhost/ibuiltin.px")

# ── A：原生 `bi_sorted` 忽略 key（键=元素）⇒ sorted_key 判红，base32/hmac 仍绿 ──
A_OLD = """        LXValue kv = px_call(keyfn, &e, 1);
        PX_KEEP(kv);       // M170 纪律：回调返回的新对象须跨 px_list_push 的扩容分配存活
        px_list_push(ks, kv);"""
A_NEW = """        /* NEGCTL-A */
        px_list_push(ks, e);"""

# ── B：解释轨 `i_builtin_sorted` 忽略 key（**必须重编 dev 解释器**才算真跑）──
B_OLD = """        var ki = 0
        while ki < n:
            let kr = i_call_value(args[1], [items[ki]], pos)
            if kr.is_err():
                return Err(kr.err())
            keys.append(kr.unwrap())
            ki += 1"""
B_NEW = """        var ki = 0
        while ki < n:
            keys.append(items[ki])   # NEGCTL-B
            ki += 1"""

# ── C：base32 解码去掉「长度/填充合法性校验」⇒ 非法输入不再 null（仅解码面红）──
#   ⚠️ 第一版桩是「去掉有效位掩码 `acc &= (1u << nbits) - 1;`」——**没有牙**：
#     实测那个掩码是**防御性**的（读取窗口恒为 [nbits, nbits+8) 且 nbits ≤ 7 ⇒
#     低位 15 位始终由移位累积正确维护）⇒ 去掉后输出逐字节不变（门当场判「未判红」）。
#     换成「去掉非法输入校验」才有牙（d12/d13/d14 从 null 变成错值）。
C_OLD = """    size_t rem = nd % 8;
    if (rem != 0 && rem != 2 && rem != 4 && rem != 5 && rem != 7) { xfree(clean); return -1; }
    if (pad > 0 && (rem == 0 || (size_t)(8 - rem) != pad)) { xfree(clean); return -1; }"""
C_NEW = """    size_t rem = nd % 8;
    (void)rem;   /* NEGCTL-C：去掉长度/填充合法性校验 ⇒ 非法输入不再 null */"""

# ── D：原生稳定性条件 `> 0` → `>= 0` ⇒ 键相等不再保持原序（仅稳定性面红）──
D_OLD = """            if (compare_values(ko->as.list.items[idx[j]], ko->as.list.items[idx[j + 1]]) > 0) {"""
D_NEW = """            if (compare_values(ko->as.list.items[idx[j]], ko->as.list.items[idx[j + 1]]) >= 0) {   /* NEGCTL-D */"""

# ── E：把 `bdata` 的每线程轮转环退回「一处共享 static」⇒ 缺陷 244 复现 ──
E_OLD = """    if (v.type == PX_STR || v.type == PX_BYTES) return v.as.obj->as.str.data;
    // M202（缺陷 244）：**每线程轮转环**（同 val_cstr；见 px_tmp_slot 注释）
    char* tmp = px_tmp_slot();
    snprintf(tmp, PX_TMPSZ, "%s", fmt_num(v));
    return tmp;
}"""
E_NEW = """    if (v.type == PX_STR || v.type == PX_BYTES) return v.as.obj->as.str.data;
    static char tmp[64];   /* NEGCTL-E */
    snprintf(tmp, sizeof(tmp), "%s", fmt_num(v));
    return tmp;
}"""

SPEC = {
    "A": (RUNTIME, A_OLD, A_NEW),
    "B": (IBUILTIN, B_OLD, B_NEW),
    "C": (RUNTIME, C_OLD, C_NEW),
    "D": (RUNTIME, D_OLD, D_NEW),
    "E": (RUNTIME, E_OLD, E_NEW),
}

def main():
    if len(sys.argv) < 2:
        print("用法：python3 stubs.py <A|B|C|D|E> [仓库根]", file=sys.stderr)
        return 2
    letter = sys.argv[1]
    if letter not in SPEC:
        print(f"未知桩号 {letter}", file=sys.stderr)
        return 2
    path, old, new = SPEC[letter]
    s = open(path, encoding="utf-8").read()
    n = s.count(old)
    if n != 1:
        print(f"NEGCTL-{letter} 打桩失败：锚点在 {path} 出现 {n} 次（期望 1）", file=sys.stderr)
        return 3
    open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
    print(f"NEGCTL-{letter} 已打桩于 {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
