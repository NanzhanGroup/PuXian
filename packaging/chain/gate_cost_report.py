#!/usr/bin/env python3
# ============================================================
# packaging/chain/gate_cost_report.py —— 全量门**成本模型**报告（M311 · B1）
# ------------------------------------------------------------
# 用途：把「分片档一轮到底把时间花在哪」变成可复算的数字，而不是感觉。
#   输入 = 一个分片日志目录（`/data/gatevm/shard-YYYYmmdd-HHMMSS`，含 shardN.times.tsv /
#          quiet.times.tsv / plan.tsv）与可选的 devbuild 台账目录（各分片自己的 /tmp）。
#   输出：① 每片门数与门耗时合计 ② 墙钟的两个分量（最长分片 / quiet 串行段）
#        ③ 平衡度（最长分片 vs 理想均分）④ **重复重建**统计（同一 (件,key) 被多片各建一次）
#        ⑤ Top-N 最贵的门
#
# 为什么要有它：M311 B 项的立项假设是「4 路分片互相重复建 devbuild 产物 ⇒ 总工作量被放大」。
#   实测（R2 · 2026-10-18）该假设**不成立**：138 次重建里只有 8 次是跨片重复（≈2 分钟）。
#   ⇒ 这正是 M104「量化表否定的子项立即降级」的场景。本脚本是那份量化的**可复算形式**，
#     以后每轮想再谈「压缩工作量」时，先跑它。
#
# 用法：
#   packaging/chain/gate_cost_report.py <shard_dir> [--tmp-root /data/gatevm] [--top 12]
# 退出码：0 正常 / 2 参数错 / 3 目录或关键文件缺失
# ============================================================
import argparse
import collections
import glob
import os
import sys


def read_times(path):
    rows = {}
    if not os.path.exists(path):
        return rows
    for line in open(path, encoding="utf-8", errors="replace"):
        p = line.rstrip("\n").split("\t")
        if len(p) < 3:
            continue
        name, st, sec = p[0], p[1], p[2]
        try:
            sec = int(sec)
        except ValueError:
            continue
        if st == "SKIP":
            continue
        rows[name] = (st, sec)
    return rows


def main():
    ap = argparse.ArgumentParser(add_help=True)
    ap.add_argument("shard_dir")
    ap.add_argument("--tmp-root", default="/data/gatevm",
                    help="各分片的 tmp 根（内有 tmp1../devbuild_stats.tsv）")
    ap.add_argument("--top", type=int, default=12)
    a = ap.parse_args()

    d = a.shard_dir
    if not os.path.isdir(d):
        print("❌ 目录不存在：%s" % d, file=sys.stderr)
        return 3
    shard_files = sorted(glob.glob(os.path.join(d, "shard[0-9]*.times.tsv")))
    if not shard_files:
        print("❌ 找不到 shardN.times.tsv（不是分片目录？）", file=sys.stderr)
        return 3

    lanes = {}
    all_gates = {}
    for f in shard_files:
        n = os.path.basename(f).split(".")[0]
        rows = read_times(f)
        lanes[n] = sum(s for _, s in rows.values())
        for g, (st, s) in rows.items():
            all_gates[g] = (n, st, s)

    quiet = read_times(os.path.join(d, "quiet.times.tsv"))
    qt = sum(s for _, s in quiet.values())

    print("══ 全量门成本模型（%s）══" % os.path.basename(d.rstrip("/")))
    print("\n① 分片（并行段）")
    for n in sorted(lanes):
        cnt = len(read_times(os.path.join(d, "%s.times.tsv" % n)))
        print("   %-7s %3d 门 · 合计 %6ds（%.1f min）" % (n, cnt, lanes[n], lanes[n] / 60))
    total = sum(lanes.values())
    mx = max(lanes.values())
    ideal = total / max(1, len(lanes))
    print("   合计 %ds · 最长 %ds · 理想均分 %ds ⇒ 不平衡损失 **%ds**（%.1f%% 的墙钟）"
          % (total, mx, ideal, mx - ideal, 100.0 * (mx - ideal) / max(1, mx + qt)))

    print("\n② quiet 串行段（分片跑完后单独跑，低负载门）")
    if quiet:
        print("   %d 门 · 合计 %ds（%.1f min）" % (len(quiet), qt, qt / 60))
        for g, (st, s) in sorted(quiet.items(), key=lambda kv: -kv[1][1])[:6]:
            print("      %-26s %5ds" % (g, s))
    else:
        print("   （无 quiet.times.tsv）")

    wall_est = mx + qt
    print("\n③ 墙钟构成：最长分片 %ds + quiet %ds ⇒ ≈ %ds（%.1f min）"
          % (mx, qt, wall_est, wall_est / 60.0))

    # ④ 重复重建（跨片同名同 key）
    reb = []
    for i in range(1, 32):
        f = os.path.join(a.tmp_root, "tmp%d" % i, "devbuild_stats.tsv")
        if not os.path.exists(f):
            continue
        for line in open(f, encoding="utf-8", errors="replace"):
            p = line.rstrip("\n").split("\t")
            if len(p) >= 4 and p[1] == "rebuild":
                reb.append((i, p[2], p[3]))
    print("\n④ devbuild 重建（B 项的立项假设）")
    if reb:
        uniq = collections.Counter((n, k) for _, n, k in reb)
        dup = sum(c - 1 for c in uniq.values() if c > 1)
        print("   重建 %d 次 · 唯一(件,key) %d 个 ⇒ **跨片重复 %d 次**（按 15s/次 ≈ %ds）"
              % (len(reb), len(uniq), dup, dup * 15))
        if dup * 15 < max(1, wall_est) * 0.03:
            print("   ⇒ 占墙钟 %.1f%%：**假设不成立**（共享缓存收益可忽略 · M104 降级）"
                  % (100.0 * dup * 15 / max(1, wall_est)))
        else:
            print("   ⇒ 占墙钟 %.1f%%：值得考虑共享产物槽"
                  % (100.0 * dup * 15 / max(1, wall_est)))
        for (n, k), c in uniq.most_common(5):
            if c > 1:
                print("      %s %s × %d 片" % (n, k, c))
    else:
        print("   ℹ️ 没有找到 devbuild 台账（%s/tmp*/devbuild_stats.tsv）" % a.tmp_root)

    print("\n⑤ 最贵的门（Top %d）" % a.top)
    for g, (n, st, s) in sorted(all_gates.items(), key=lambda kv: -kv[1][2])[:a.top]:
        print("   %-28s %6ds  %s  (%s)" % (g, s, st, n))
    return 0


if __name__ == "__main__":
    sys.exit(main())
