#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
packaging/chain/quiet_archive.py —— quiet 档「归档器」（M383 · 2/3）

背景（实测，2026-11-09）：
  分片墙钟已经从 90min 砍到 49min，但 quiet 串行尾 16 门合计 **2428s = 40.5min**
  （/data/gatevm/shard-20260911.../quiet.times.tsv，M382 第四轮），瓶颈换了位置。
  其中 6 门实测 ≥180s（m244 514s · m302 285s · m291 228s · m208 205s · m296 190s · m231 183s），
  但它们**当初进 quiet 的理由不是「快」，而是「在分片并发下判据会假红」** ——
  所以「慢就得挪回分片」是错读。真正的判据是：**这门对「轻并发」敏不敏感**。

本归档器做三件事，全部可复核：
  1) 读上一轮实测 `times.tsv`，按阈值（默认 180s）把 quiet 门分成两档候选：
       · 并行档（par）：慢，但经实测可在「K 路并行 · 每路 2 核 · JOBS=2」下保持绿
       · 独占档（solo）：判据前提取决于「机器空闲」（性能比/时延窗口/端口窗口/GC 停顿）⇒ 必须串行
  2) 独占集的**白名单在数据里**（quiet_exclusive.txt，每条带理由），不在代码里 ——
     理由必须可审计，不允许「因为我说它敏感」。
  3) `--check` 是**漂移判据**：当前 quiet_lanes.txt 与「按实测重算的结果」不一致 ⇒ rc=3。
     负控（可复现）：把某门实测耗时人为改小/改大 ⇒ `--check` 必须报「该挪/该回」并 rc=3。

用法：
  quiet_archive.py --times <tsv> [--quiet <quiet_needed.txt>] [--lanes <quiet_lanes.txt>]
                   [--exclusive <quiet_exclusive.txt>] [--threshold 180] [--n 3]
                   [--write] [--check] [--selftest]
退出码：0 = 一致/已写 · 3 = 漂移（--check）· 4 = 参数或输入缺失
"""
import argparse
import glob
import os
import sys

BASE_DEFAULT = os.environ.get("PX_GATEVM", "/data/gatevm")


def read_gates(path):
    """quiet_needed.txt：剥行内注释、按空白取第一列"""
    out = []
    if not os.path.exists(path):
        return out
    for line in open(path, encoding="utf-8"):
        line = line.split("#")[0].strip()
        if line:
            out.append(line.split()[0])
    return out


def read_lanes(path):
    """quiet_lanes.txt：`<gate> <lane>`；lane 是正整数。返回 (dict, 原顺序)"""
    m, order = {}, []
    if not os.path.exists(path):
        return m, order
    for line in open(path, encoding="utf-8"):
        line = line.split("#")[0].strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        g, ln = parts[0], parts[1]
        if not ln.isdigit():
            continue
        m[g] = int(ln)
        order.append(g)
    return m, order


def read_exclusive(path):
    """quiet_exclusive.txt：`<gate> <理由>`；返回 dict gate→理由"""
    m = {}
    if not os.path.exists(path):
        return m
    for line in open(path, encoding="utf-8"):
        line = line.split("#")[0].strip()
        if not line:
            continue
        parts = line.split(None, 1)
        m[parts[0]] = parts[1].strip() if len(parts) > 1 else "（未写理由 ⇒ 视为登记不全）"
    return m


def read_times(path):
    t = {}
    if not os.path.exists(path):
        return t
    for line in open(path, encoding="utf-8"):
        p = line.rstrip("\n").split("\t")
        if len(p) < 3:
            continue
        try:
            t[p[0]] = int(float(p[2]))
        except ValueError:
            continue
    return t


def pick_times(base, quiet):
    """按「新 → 旧」选实测源：**只接受覆盖全部 quiet 门**的表（缺陷 678）。

    为什么要这条判据（2026-11-09 14:1x 实证）：
      我为「车道能不能并行」跑了一次**限范围探针**（gates.txt 只有 4 个 quiet 门），
      shard_run.sh 照常写出 `quiet.times.tsv`（4 行）。
      而校验器旧口径 = 「取 mtime 最新的 quiet.times.tsv」⇒ 拿这张残表去重算 16 门
      ⇒ 12 门「无实测 ⇒ 保守独占」⇒ 与已归档 lanes 不一致 ⇒ 假报「归档漂移」：
        同一份归档，--write 说「独占 8 门 3422s」、--check 说「17s / 车道 180s」。
      结论：不是归档错了，是**校验器选错了实测源**。
    判据：完整链的 quiet 表必须覆盖 quiet_needed 的每一门（残表一律拒，不许静默取证）。
    返回 (path|None, rejected:[(path, missing_gates)], stamped:bool)
    """
    cands = []
    for f in glob.glob(os.path.join(base, "shard-*", "quiet.times.tsv")):
        try:
            cands.append((os.path.getmtime(f), f))
        except OSError:
            continue
    cands.sort(reverse=True)
    rejected = []
    for _mt, f in cands:
        t = read_times(f)
        miss = [g for g in quiet if g not in t]
        if not miss:
            stamp = os.path.join(os.path.dirname(f), "CHAIN-COMPLETE.txt")
            return f, rejected, os.path.exists(stamp)
        rejected.append((f, miss))
    return None, rejected, False


def lpt(gates, times, n):
    """LPT 装箱：按耗时降序塞进当前最轻的 lane（确定性：并耗时并列按门名）"""
    lanes = [[] for _ in range(n)]
    load = [0] * n
    for g in sorted(gates, key=lambda x: (-times.get(x, 0), x)):
        i = load.index(min(load))
        lanes[i].append(g)
        load[i] += times.get(g, 0)
    return lanes, load


def compute(quiet, times, excl, threshold, n):
    """返回 (solo, lanes, load, decisions) —— decisions: gate → (tier, 理由)

    规则（M383 实测后定稿，2026-11-09）：
      R1 **白名单 ⇒ 独占**：quiet_exclusive.txt 里的门一律串行（哪怕 900s）——
         它们的判据前提取决于「机器空闲」（性能比 / 时延窗口 / 重编窗口 / 端口窗口），
         并行跑不是慢，是**判据失效**。
      R2 **其余 ⇒ 并行车道**（LPT）——包括小门：小门搭车道是免费的（实测 m120 24s、
         m312 25s 在车道里与单跑同速），旧版「<阈值就留串行」把免费收益丢掉了。
      R3 **无实测 ⇒ 保守独占**：没有耗时就没有装箱依据，宁可串行也不瞎排。
      R4 阈值只做**宣示**（哪些门的移档真正省墙钟），不作为档位判据 ——
         若拿它当判据，下一轮耗时一抖动档位就跳，归档表变成噪声。
    """
    solo, par = [], []
    decisions = {}
    for g in quiet:
        t = times.get(g)
        if g in excl:
            solo.append(g)
            why = "独占档（判据要求空闲机）：" + excl[g]
            if t is not None and t >= threshold:
                why += " · 实测 %ss ≥%s 但**不移**" % (t, threshold)
            decisions[g] = ("solo", why)
            continue
        if t is None:
            solo.append(g)
            decisions[g] = ("solo", "无实测耗时 ⇒ 保守留独占档（下轮回灌后可归档）")
            continue
        par.append(g)
        tag = "实测 %ss" % t + (" ≥%s ⇒ 移档能省墙钟" % threshold if t >= threshold else " <阈值，搭车道免费")
        decisions[g] = ("par", tag + " ⇒ 并行档")
    lanes, load = lpt(par, times, n)
    return solo, lanes, load, decisions


def render_lanes(lanes):
    lines = []
    for i, gates in enumerate(lanes, 1):
        for g in gates:
            lines.append("%s %d" % (g, i))
    return "\n".join(lines) + ("\n" if lines else "")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--times", default=os.path.join(BASE_DEFAULT, "times_selftuned.tsv"))
    ap.add_argument("--quiet", default=os.path.join(BASE_DEFAULT, "quiet_needed.txt"))
    ap.add_argument("--lanes", default=os.path.join(BASE_DEFAULT, "quiet_lanes.txt"))
    ap.add_argument("--exclusive", default=os.path.join(BASE_DEFAULT, "quiet_exclusive.txt"))
    ap.add_argument("--threshold", type=int, default=180)
    ap.add_argument("--n", type=int, default=3)
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--base", default=BASE_DEFAULT, help="链目录根（shard-* 所在处）")
    ap.add_argument("--pick", action="store_true", help="只挑实测源（按覆盖度，不重算归档）")
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--selftest-pick", action="store_true")
    a = ap.parse_args()

    if a.selftest:
        return selftest()
    if a.selftest_pick:
        return selftest_pick()

    quiet = read_gates(a.quiet)
    if not quiet:
        print("❌ quiet 清单为空或读不到：%s" % a.quiet, file=sys.stderr)
        return 4

    if a.pick:
        p, rej, st = pick_times(a.base, quiet)
        for f, miss in rej:
            print("PICK-REJECTED=%s miss=%d %s" % (f, len(miss), ",".join(miss)))
        print("PICK-REJECTED-TOTAL=%d" % len(rej))
        if not p:
            print("PICK-STATUS=NONE")
            print("❌ 没有「覆盖全部 %d 个 quiet 门」的实测表（%d 份残表已拒）" % (len(quiet), len(rej)),
                  file=sys.stderr)
            return 3
        print("PICK-STATUS=FULL")
        print("PICK-FILE=%s" % p)
        print("PICK-STAMP=%s" % ("yes" if st else "no"))
        return 0

    times = read_times(a.times)
    if not times:
        print("❌ 耗时表为空或读不到：%s（没有实测就不许归档）" % a.times, file=sys.stderr)
        return 4
    excl = read_exclusive(a.exclusive)
    cur, _order = read_lanes(a.lanes)

    solo, lanes, load, decisions = compute(quiet, times, excl, a.threshold, a.n)

    print("═══ quiet 归档（阈值 %ss · 车道 %d · 门 %d）═══" % (a.threshold, a.n, len(quiet)))
    for g in quiet:
        tier, why = decisions[g]
        print("  %-28s %5ss  %-5s %s" % (g, times.get(g, "-"), tier, why))
    for i, gates in enumerate(lanes, 1):
        print("  车道%d：%3ds · %s" % (i, load[i - 1], " ".join(gates) or "（空）"))
    solo_sum = sum(times.get(g, 0) for g in solo)
    par_wall = max(load) if load else 0
    print("  ⇒ 独占档合计 %ds（%.1f min）· 并行档墙钟下界 %ds（%.1f min）· 归档后 quiet 尾 ≈ %.1f min"
          % (solo_sum, solo_sum / 60.0, par_wall, par_wall / 60.0, (solo_sum + par_wall) / 60.0))

    # ── 漂移判据：当前 quiet_lanes.txt ⇄ 重算结果 ──
    want = {}
    for i, gates in enumerate(lanes, 1):
        for g in gates:
            want[g] = i
    drift_tier = [g for g in quiet if (g in want) != (g in cur)]
    drift_lane = [g for g in quiet if g in want and g in cur and want[g] != cur[g]]
    extra = [g for g in cur if g not in quiet]

    if a.check:
        if drift_tier or extra:
            print("❌ 归档漂移：档位变化 %d 门 %s%s" %
                  (len(drift_tier), drift_tier, (" · 清单外条目 %s" % extra) if extra else ""), file=sys.stderr)
            print("   修法：quiet_archive.py --write（把归档结果落盘）", file=sys.stderr)
            return 3
        if drift_lane:
            print("  ⚠ 车道号变化（不影响判据，仅提示重排）：%s" % drift_lane)
        print("  ✅ quiet 归档一致（QUIET-ARCHIVE-OK）")
        return 0

    if a.write:
        hdr = ("# quiet_lanes.txt —— quiet 档的**并行车道**归属（M383 · 2/3 归档器产物，别手改）\n"
               "#   生成：packaging/chain/quiet_archive.py --write\n"
               "#   口径：<gate> <lane>；不在此表的 quiet 门 = 独占串行档。\n"
               "#   独占白名单与理由见 quiet_exclusive.txt（每条都要能说清判据前提）。\n"
               "#   判据：quiet_archive.py --check 一致才算归档稳定（漂移 ⇒ rc=3）。\n")
        with open(a.lanes, "w", encoding="utf-8") as f:
            f.write(hdr + render_lanes(lanes))
        print("  ✅ 已写 %s（并行 %d 门 / 独占 %d 门）" % (a.lanes, len(want), len(solo)))
    return 0


def selftest():
    """自带正负控：归档器只有能「发现变化」才算判据"""
    quiet = ["g_fast", "g_slow", "g_excl", "g_unknown"]
    excl = {"g_excl": "判据要求空闲机（自测样例）"}
    base = {"g_fast": 30, "g_slow": 300, "g_excl": 400}
    ok = True

    def chk(name, cond):
        nonlocal ok
        print(("  ✅ " if cond else "  ❌ ") + name)
        if not cond:
            ok = False

    solo, lanes, load, dec = compute(quiet, base, excl, 180, 3)
    par_all = [g for l in lanes for g in l]
    chk("慢门进并行档", "g_slow" in par_all)
    chk("快门也搭车道（免费）", "g_fast" in par_all)
    chk("白名单 900s 也不进车道（判据优先于耗时）", "g_excl" in solo and "g_excl" not in par_all)
    chk("无实测 ⇒ 保守留独占", "g_unknown" in solo)

    # 负控 A：把门从白名单移出 ⇒ 必须进车道
    solo2, lanes2, _l2, _d2 = compute(quiet, base, {}, 180, 3)
    chk("负控A：白名单移出 ⇒ 进并行档", "g_excl" in [g for l in lanes2 for g in l])

    # 负控 B：把快门加进白名单 ⇒ 必须回独占
    e3 = dict(excl); e3["g_fast"] = "自测：人为加入白名单"
    solo3, lanes3, _l3, _d3 = compute(quiet, base, e3, 180, 3)
    chk("负控B：加入白名单 ⇒ 回独占档", "g_fast" in solo3 and "g_fast" not in [g for l in lanes3 for g in l])

    # 装箱：并行门应尽量均分（最大 load ≤ 总量）
    t4 = {"a": 100, "b": 100, "c": 100, "d": 100}
    _s4, l4, load4, _d4 = compute(list(t4), t4, {}, 0, 2)
    chk("装箱均分（2 车道 4×100 ⇒ 200/200）", load4 == [200, 200])
    print("QUIET-ARCHIVE-SELFTEST-%s" % ("OK" if ok else "FAIL"))
    return 0 if ok else 1


def selftest_pick():
    """选源判据自证：**必须拒掉「最新的残表」** —— 缺陷 678 的现场原样搬进来"""
    import shutil
    import tempfile
    ok = True

    def chk(name, cond):
        nonlocal ok
        print(("  ✅ " if cond else "  ❌ ") + name)
        if not cond:
            ok = False

    quiet = ["g_a", "g_b", "g_c", "g_d"]
    full = "".join("%s\tOK\t%d\t-\n" % (g, 10 * (i + 1)) for i, g in enumerate(quiet))
    partial = "g_a\tOK\t10\t-\n"
    d = tempfile.mkdtemp(prefix="qarch-pick-")
    try:
        old = os.path.join(d, "shard-20260101-000000")
        new = os.path.join(d, "shard-20260102-000000")
        os.makedirs(old)
        os.makedirs(new)
        open(os.path.join(old, "quiet.times.tsv"), "w", encoding="utf-8").write(full)
        open(os.path.join(new, "quiet.times.tsv"), "w", encoding="utf-8").write(partial)
        os.utime(os.path.join(old, "quiet.times.tsv"), (1000, 1000))
        os.utime(os.path.join(new, "quiet.times.tsv"), (2000, 2000))

        p, rej, st = pick_times(d, quiet)
        chk("正控：选到完整表（更旧的那一轮）", p == os.path.join(old, "quiet.times.tsv"))
        chk("正控：不完整的**最新**表被拒（缺陷 678 现场）",
            len(rej) == 1 and rej[0][0] == os.path.join(new, "quiet.times.tsv"))
        chk("正控：缺门清单可指认（3 门）", (len(rej[0][1]) == 3) if rej else False)
        chk("负控A：未写完成戳 ⇒ stamped=False（不许假装有出处）", st is False)

        open(os.path.join(old, "CHAIN-COMPLETE.txt"), "w", encoding="utf-8").write("stamp=full-basis\n")
        p2, _r2, st2 = pick_times(d, quiet)
        chk("负控B：有戳 ⇒ stamped=True 且仍选完整表", st2 is True and p2 == os.path.join(old, "quiet.times.tsv"))

        shutil.rmtree(old)
        p3, rej3, _st3 = pick_times(d, quiet)
        chk("负控C：只剩残表 ⇒ 返回 None（宁可 SKIP，不许拿残表断言）", p3 is None and len(rej3) == 1)

        # 负控D：残表重算 ⇒ 档位被误判（证明「选源」不是装饰）
        full_t = {g: 10 * (i + 1) for i, g in enumerate(quiet)}
        part_t = {"g_a": 10}
        solo_f, _l_f, _lf_f, _d_f = compute(quiet, full_t, {}, 180, 2)
        solo_p, _l_p, _lf_p, _d_p = compute(quiet, part_t, {}, 180, 2)
        chk("负控D：拿残表重算 ⇒ ≥3 门被误判为独占（旧口径假红机制）", len(solo_p) >= len(solo_f) + 3)
    finally:
        shutil.rmtree(d, ignore_errors=True)

    print("PICK-SELFTEST-%s" % ("OK" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
