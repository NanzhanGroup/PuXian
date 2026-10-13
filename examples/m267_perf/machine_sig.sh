#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m267_perf/machine_sig.sh —— 机器签名（**单一实现**）
#
# 为什么抽出来（M305）：`verify.sh`（选档）与 `check_spec_consistency.sh`
#   （「档 ⇄ 实机」自洽）都要用同一个签名 ⇒ 不抽就是**同一条语义两份实现**
#   —— 那是本仓反复踩过的形状（M215 缺陷 305 / M230 缺陷 345 / M232 缺陷 347）。
#
# 两层：
#   machine_model() —— **原始 CPU 型号串**（档头注「生成机：」与 collect 的 env.cpu_model 用）
#   machine_sig()   —— 由前者归一化出的**档文件名**
#   为什么分两层（M307）：型号串是**档头注/JSON 里要如实记录**的东西，签名是**文件名**；
#   两者若各写一遍取法，就是「同一条语义两份实现」（本文件头注已说明该形状的代价）。
#
# 型号来源（确定性 · 只读 · 无外部依赖）：先 `/proc/cpuinfo` 的 `model name`；
#   取不到（aarch64）⇒ 回退 `lscpu` 的 `Model name:`；再取不到 ⇒ `unknown-<uname -m>`。
#
# 归一化规则：
#   小写 → 去 (r)/(tm) → 去 "N-Core" → 去 "@…GHz" → 去 cpu/processor 词 →
#   非字母数字折叠成 '-' → 去首尾/连续 '-'
#
# 实测（本仓真实见过的四台机器）：
#   `AMD EPYC 7K62 48-Core Processor`                → amd-epyc-7k62
#   `Intel(R) Xeon(R) CPU E5-2682 v4 @ 2.50GHz`      → intel-xeon-e5-2682-v4
#   `Intel(R) Xeon(R) Platinum 8269CY CPU @ 2.50GHz` → intel-xeon-platinum-8269cy
#   （aarch64 无 `model name`，走 lscpu 回退）`Neoverse-N2` → neoverse-n2
#   （CI runner / 陌生机器 ⇒ 档通常不存在 ⇒ 见 verify.sh 的「合成档」回退）
#
# ⚠️ 架构差异（M307 首台 aarch64 真机照出）：
#   x86 的 /proc/cpuinfo 有 `model name`；**aarch64 没有**（ARM 用 `CPU implementer` /
#   `CPU part`，如 0x41 / 0xd49 = Neoverse-N2）。若只读 /proc/cpuinfo，aarch64 会退化成
#   `unknown-aarch64` ⇒ **所有 ARM 机型共用一个档**（档位机制对 ARM 失效）。
#   ⇒ 回退 `lscpu` 的 `Model name:`（x86/arm 都有）。**x86 路径一字未动**（先取 model name）。
#
# 用法：  . "$(dirname "$0")/machine_sig.sh" ; sig=$(machine_sig) ; model=$(machine_model)
# 只定义函数，**不执行任何动作**（source 安全）。
# ══════════════════════════════════════════════════════════════
machine_model() {
    local m
    m=$(grep -m1 'model name' /proc/cpuinfo 2>/dev/null | sed 's/.*: //')
    # aarch64 回退：/proc/cpuinfo 无 `model name` ⇒ 取 lscpu 的 `Model name:`
    #   （`sed -n 's/^Model name:…//p'` 锚定行首 ⇒ 不会误取 `BIOS Model name:`）
    if [ -z "$m" ] && command -v lscpu >/dev/null 2>&1; then
        m=$(lscpu 2>/dev/null | sed -n 's/^Model name:[[:space:]]*//p' | head -1)
    fi
    [ -n "$m" ] || m="unknown-$(uname -m)"
    printf '%s' "$m"
}

machine_sig() {
    machine_model | tr 'A-Z' 'a-z' \
      | sed -E 's/\(r\)|\(tm\)//g; s/[0-9]+-core//g; s/@.*//g; s/\b(cpu|processor)\b//g' \
      | sed -E 's/[^a-z0-9]+/-/g; s/^-+//; s/-+$//; s/-+/-/g'
}
