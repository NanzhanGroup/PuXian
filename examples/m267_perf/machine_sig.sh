#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m267_perf/machine_sig.sh —— 机器签名（**单一实现**）
#
# 为什么抽出来（M305）：`verify.sh`（选档）与 `check_spec_consistency.sh`
#   （「档 ⇄ 实机」自洽）都要用同一个签名 ⇒ 不抽就是**同一条语义两份实现**
#   —— 那是本仓反复踩过的形状（M215 缺陷 305 / M230 缺陷 345 / M232 缺陷 347）。
#
# 归一化规则（确定性 · 无外部依赖 · 只读 /proc/cpuinfo）：
#   小写 → 去 (r)/(tm) → 去 "N-Core" → 去 "@…GHz" → 去 cpu/processor 词 →
#   非字母数字折叠成 '-' → 去首尾/连续 '-'
#
# 实测（本仓真实见过的三台机器）：
#   `AMD EPYC 7K62 48-Core Processor`                → amd-epyc-7k62
#   `Intel(R) Xeon(R) CPU E5-2682 v4 @ 2.50GHz`      → intel-xeon-e5-2682-v4
#   `Intel(R) Xeon(R) Platinum 8269CY CPU @ 2.50GHz` → intel-xeon-platinum-8269cy
#   （CI runner / 陌生机器 ⇒ 档通常不存在 ⇒ 见 verify.sh 的「合成档」回退）
#
# 用法：  . "$(dirname "$0")/machine_sig.sh" ; sig=$(machine_sig)
# 只定义函数，**不执行任何动作**（source 安全）。
# ══════════════════════════════════════════════════════════════
machine_sig() {
    local m
    m=$(grep -m1 'model name' /proc/cpuinfo 2>/dev/null | sed 's/.*: //')
    [ -n "$m" ] || m="unknown-$(uname -m)"
    printf '%s' "$m" | tr 'A-Z' 'a-z' \
      | sed -E 's/\(r\)|\(tm\)//g; s/[0-9]+-core//g; s/@.*//g; s/\b(cpu|processor)\b//g' \
      | sed -E 's/[^a-z0-9]+/-/g; s/^-+//; s/-+$//; s/-+/-/g'
}
