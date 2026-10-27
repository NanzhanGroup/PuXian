#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
selfhost/audit_gate_flakiness.py —— 门的「判据时序敏感性」静态盘点工具（M341 · 台账第 1 步）
================================================================================================
⚠️ 这不是门（不判红、不进全量门、不进 ci.yml）。它产出一份**可复核的名单**，供人决定先改哪一个。

为什么要有它（M340 台账第三节的结论）：
    7 轮全量门只 1 轮全绿，每次红都换一个门、同树单跑必绿。已定性的 6 条（588/589/591/594/595/596）
    共性有两条：① 判据的**观测窗口 < 现象的随机性**（单轮取样 / 4 秒窗口 / 就绪口径不等于被测通道）；
              ② 同一门里对同一事实**多次独立读取**。
    存量盘点里「含负控的已注册门」有 181 个 —— 面太大，逐个盲改就是逐个打补丁。
    ⇒ 先量化：把它们分成三档，只对人真会踩的那一档动手。

三档定义（唯一判据，别的都不算）
--------------------------------
    H（高风险）  ：**拿时间/CPU 类量下结论**，且**没有任何多轮取样**结构 ⇒ 单次观测定生死
    M（已缓解）  ：同样拿时间/CPU 类量下结论，但存在**多轮取样**结构（轮次变量 / seq 小循环 / 任一命中）
    D（确定性）  ：断言里不出现时间/CPU 类量 ⇒ 本工具认为它不受观测窗口影响

    「时间/CPU 类量」= 断言行里出现 ticks / utime / elapsed / SECONDS / ms / spin / retry / attempt / dur / cpu。
    「多轮取样」= 出现 ROUNDS/REPS/N_ROUND 类变量赋值，或 `for <x> in $(seq 1 N)` 且 N ≤ 10（>10 视为轮询等待，不算取样）。

诚实边界（必读，别把本工具的结论当定性）
----------------------------------------
    ① 这是**静态**扫描，读的是源码文本，不是运行时行为。H 档 ≠ 一定会偶发；D 档 ≠ 一定安全
       （比如日志 READY ≠ 端口可连的缺陷 589，断言行里一个时间量都没有，本工具**看不出来**）。
    ② M 档只说明「有多轮结构」，**不代表取样次数够**（4 次也可能个个踩空）；次数是否足够要人看。
    ③ 因此本工具**只产名单不下结论**；任何「因为它在 D 档所以不用管」的用法都是误用。

用法
----
    audit_gate_flakiness.py                     # 扫全量门注册清单（gates.registry.sh）
    audit_gate_flakiness.py --all               # 再并上 examples/*/verify.sh 全量
    audit_gate_flakiness.py --gate examples/m245_tls_cert_reload/verify.sh   # 单门详查
    audit_gate_flakiness.py --json out.json     # 机器可读
    audit_gate_flakiness.py --selftest          # 负控：已知案例档位必须一致，否则 rc≠0
"""

import argparse
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REGISTRY = os.path.join(ROOT, "selfhost", "gates.registry.sh")

# ── 判据模式（全部只匹配**代码行**，注释行不参与tier判定）─────────────────────
RE_ASSERT = re.compile(r"^\s*(chk|expect|assert|check|verify)\b", re.I)
TIME_WORDS = ("ticks", "utime", "elapsed", "seconds", "spin", "retry",
              "attempt", "cpu", "_ms", " ms", "dur")
RE_ROUNDS_VAR = re.compile(r"\b(ROUNDS|REPS|N_ROUND\w*|ROUND_\w+|MAX_ROUNDS)\s*=")
RE_SEQ_LOOP = re.compile(r"for\s+\w+\s+in\s+\$\(seq\s+1\s+(\d+)\)")
# `for r in 1 2 3;` —— M341 实测：m319 的 3 轮取样就是这个写法，首版判据只认 seq ⇒ 漏判（selftest 抓到）
RE_LITERAL_ROUNDS = re.compile(r"for\s+\w+\s+in\s+((?:[0-9]+\s+){1,9}[0-9]+)\s*(;|$)")
# `for ((r=0; r<3; r++))`
RE_CSTYLE_ROUNDS = re.compile(r"for\s*\(\(\s*\w+\s*=\s*[0-9]+\s*;\s*\w+\s*<\s*(\d+)\s*;")
# `K=${M120_ISO_REPEAT:-5}` + `for i in $(seq 1 "$K")` —— m120 的重复次数就是这个写法（首版又漏了）
RE_SMALL_VAR_DEF = re.compile(r"^\s*(\w+)=\$\{\w+:-(\d+)\}")
RE_SEQ_VAR = re.compile(r"for\s+\w+\s+in\s+\$\(seq\s+1\s+\"?\$(\w+)\"?\)")
RE_ANY_HIT = re.compile(r"任一命中|至少\s*1\s*轮|hit\s*=\s*1|任一次")
RE_PROBE_TICKS = re.compile(r"/proc/[^\s]*/stat|utime")
RE_SLEEP_TIMEOUT = re.compile(r"\b(sleep\s+[0-9]|timeout\s+[0-9]|--max-time)")
RE_FLAKY_WORD = re.compile(r"偶发|竞态|概率|随机|flake|race")
# ── 第二轴（M341 实测加的）：6 个已定性案例里有 5 个**根本不是时间量** ─────────────
#   588/591/594/596 的病是「对**概率性现象只取一次样**」，589 是「就绪通道≠被测通道」。
#   ⇒ 光看时间量会漏掉绝大多数；必须同时看「有没有多轮结构」「现象是不是概率性的」。
RE_PROB_PHENOMENON = re.compile(
    r"并发|竞态|概率|随机|race|timeout|超时|信号|SIG[A-Z]+|EOF|抢占|调度|"
    r"spawn|隔离|isolate|重试|retry", re.I)
RE_RETRY_STRUCT = re.compile(r"--retry|\bretry\b|\battempts?\b", re.I)
#   ⚠️ 首版这里写了 `for \w+ in $(seq`，把 m245 的 `seq 1 150` 轮询等就绪也算成「多轮取样」⇒ 已知案例漏判。
#      轮询 ≠ 取样：seq 的 N 大小才分得清（已在 RE_SEQ_LOOP 里按 ≤10 判）。
# 「驾驭活进程」信号：门自己拉起进程/服务，再去读它的运行时状态 —— 这类判据才可能踩时序随机性
RE_LIVE = re.compile(r"\$!(\s|$)|kill\s+-[A-Z]|/proc/|wait\s+\$|connect_ex|nc\s+-z|ss\s+-[a-z]|curl\s"
                     r"|start_server|wait_port\s|\s&\s*$")


def strip_comment(line):
    """去掉整行注释；行内 # 之后的也算注释（保守：不动引号里的 #，用最简规则）。"""
    s = line.lstrip()
    if s.startswith("#"):
        return ""
    # 行内注释：只处理 ' #' 形式，避免误伤 '#' 出现在字符串里的少数情况
    idx = line.find(" #")
    return line[:idx] if idx >= 0 else line


def parse_registry(path):
    """从 gates.registry.sh 抽出 (门名, 脚本路径)。格式：run <名> bash <路径> / run <名> python3 <路径>。"""
    out = []
    if not os.path.isfile(path):
        return out
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            if not line.startswith("run "):
                continue
            parts = line.split()
            if len(parts) < 4:
                continue
            name, _interp, script = parts[1], parts[2], parts[3]
            if not script.endswith((".sh", ".py")):
                continue
            out.append((name, script))
    return out


def analyze(path):
    """返回该脚本的静态特征 + 档位。"""
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            raw = f.readlines()
    except OSError as e:
        return {"path": path, "error": str(e), "tier": "?"}

    code, comments = [], []
    for i, line in enumerate(raw, 1):
        c = strip_comment(line)
        if c.strip():
            code.append((i, c))
        else:
            comments.append((i, line))

    asserts = [(i, t) for i, t in code if RE_ASSERT.match(t)]
    timed_asserts = [(i, t) for i, t in asserts
                     if any(w in t.lower() for w in TIME_WORDS)]
    probe_ticks = [(i, t) for i, t in code if RE_PROBE_TICKS.search(t)]
    sleeps = [(i, t) for i, t in code if RE_SLEEP_TIMEOUT.search(t)]

    rounds_var = [(i, t.strip()) for i, t in code if RE_ROUNDS_VAR.search(t)]
    small_seq = []
    small_vars = {}
    for i, t in code:
        m = RE_SMALL_VAR_DEF.match(t)
        if m and int(m.group(2)) <= 10:
            small_vars[m.group(1)] = i
    for i, t in code:
        m = RE_SEQ_LOOP.search(t)
        if m and int(m.group(1)) <= 10:
            small_seq.append((i, t.strip()))
            continue
        m = RE_SEQ_VAR.search(t)
        if m and m.group(1) in small_vars:
            small_seq.append((i, t.strip() + f"   [轮次变量 ${m.group(1)} 定义于 L{small_vars[m.group(1)]}]"))
            continue
        m = RE_LITERAL_ROUNDS.search(t)
        if m and 2 <= len(m.group(1).split()) <= 10:
            small_seq.append((i, t.strip()))
            continue
        m = RE_CSTYLE_ROUNDS.search(t)
        if m and int(m.group(1)) <= 10:
            small_seq.append((i, t.strip()))
    any_hit = [(i, t.strip()) for i, t in comments if RE_ANY_HIT.search(t)]

    negctl = sum(1 for _, t in comments if "负控" in t)
    flaky_words = [i for i, t in comments if RE_FLAKY_WORD.search(t)]

    has_time_judgement = bool(timed_asserts) or bool(probe_ticks)
    has_rounds = bool(rounds_var) or bool(small_seq)

    if not has_time_judgement:
        tier = "D"
    elif has_rounds:
        tier = "M"
    else:
        tier = "H"

    # 第二轴：单发取样（single-shot）。只在「有负控 + 现象是概率性的 + 无任何多轮/重试结构」时亮。
    retry_struct = [(i, t.strip()) for i, t in code if RE_RETRY_STRUCT.search(t)]
    prob_lines = [i for i, t in comments + code if RE_PROB_PHENOMENON.search(t)]
    live_lines = [(i, t.strip()) for i, t in code if RE_LIVE.search(t)]
    has_rounds_any = has_rounds or bool(retry_struct)
    single_shot = bool(negctl) and not has_rounds_any and len(prob_lines) >= 3
    # 窄口径（真会踩时序的那一档）：单发 + 自己驾驭活进程。6 个已定性案例里 4 个落在这里。
    live_single = single_shot and len(live_lines) >= 3

    # 证据行：优先给「拿时间量下断言」的那一行；没有就给 ticks 读取行
    evidence = timed_asserts[:2] or probe_ticks[:2]
    return {
        "path": os.path.relpath(path, ROOT),
        "tier": tier,
        "asserts": len(asserts),
        "timed_asserts": [f"L{i}: {t.strip()[:160]}" for i, t in timed_asserts[:3]],
        "probe_ticks": [f"L{i}: {t.strip()[:120]}" for i, t in probe_ticks[:2]],
        "rounds": [f"L{i}: {t}" for i, t in rounds_var[:2]] + [f"L{i}: {t}" for i, t in small_seq[:2]],
        "any_hit_declared": [f"L{i}: {t[:120]}" for i, t in any_hit[:2]],
        "negctl_mentions": negctl,
        "single_shot": single_shot,
        "live_single": live_single,
        "live_lines": len(live_lines),
        "prob_lines": len(prob_lines),
        "retry_struct": [f"L{i}: {t[:100]}" for i, t in retry_struct[:2]],
        "sleep_or_timeout_lines": len(sleeps),
        "flaky_word_lines": list(flaky_words),
        "evidence": [f"L{i}: {t.strip()[:160]}" for i, t in evidence],
    }


def collect(args):
    items = []
    if args.gate:
        return [("(single)", args.gate)]
    items.extend(parse_registry(REGISTRY))
    if args.all:
        seen = {p for _, p in items}
        for d in sorted(os.listdir(os.path.join(ROOT, "examples"))):
            p = os.path.join("examples", d, "verify.sh")
            if os.path.isfile(os.path.join(ROOT, p)) and p not in seen:
                items.append((d, p))
    return items


def selftest():
    """负控：已知三个案例的档位必须一致。不一致 ⇒ rc=1（工具的判据被自己的启发式骗了）。"""
    cases = [
        ("examples/m319_ws_eof_spin/verify.sh", "M", "缺陷594：已改 3 轮取样 ⇒ 必须判 M"),
        ("examples/m245_tls_cert_reload/verify.sh", "H", "缺陷596：单轮未改 ⇒ 必须判 H"),
        ("examples/m190_*/verify.sh", None, "(占位，跳过)"),
    ]
    fail = 0
    for path, want, why in cases:
        if want is None:
            continue
        full = os.path.join(ROOT, path)
        if not os.path.isfile(full):
            print(f"⚠️  跳过（不存在）：{path}")
            continue
        got = analyze(full)["tier"]
        ok = got == want
        print(f"{'✅' if ok else '❌'} {path}: 期望 {want} 实得 {got} —— {why}")
        if not ok:
            fail += 1

    # 合成样本负控（不依赖仓库现状，验证「有轮次 ⇒ M」「无轮次 ⇒ H」这两条判定本身）
    import tempfile
    fixtures = [
        ("tier_H.sh",
         '#!/bin/bash\nchk "x" "[ "$(utime_of $pid)" -lt 120 ]"\n', "H"),
        ("tier_M.sh",
         '#!/bin/bash\nROUNDS=3\nfor r in $(seq 1 $ROUNDS); do\nchk "x" "[ "$(utime_of $pid)" -lt 120 ]"\ndone\n', "M"),
        ("tier_D.sh",
         '#!/bin/bash\nchk "x" "[ "$a" = "$b" ]"\n', "D"),
    ]
    with tempfile.TemporaryDirectory() as td:
        for name, body, want in fixtures:
            p = os.path.join(td, name)
            with open(p, "w", encoding="utf-8") as f:
                f.write(body)
            got = analyze(p)["tier"]
            ok = got == want
            print(f"{'✅' if ok else '❌'} 合成 {name}: 期望 {want} 实得 {got}")
            if not ok:
                fail += 1

    print("=== SELFTEST %s ===" % ("OK" if fail == 0 else f"FAIL({fail})"))
    return 1 if fail else 0


def main():
    ap = argparse.ArgumentParser(description="门的判据时序敏感性静态盘点（不是门）")
    ap.add_argument("--all", action="store_true", help="并上 examples/*/verify.sh 全量")
    ap.add_argument("--gate", help="单门详查")
    ap.add_argument("--json", help="JSON 输出路径")
    ap.add_argument("--top", type=int, default=25, help="Markdown 里列多少个 H 档（默认 25）")
    ap.add_argument("--selftest", action="store_true", help="跑已知案例负控")
    args = ap.parse_args()

    if args.selftest:
        return selftest()

    items = collect(args)
    results = []
    for name, rel in items:
        full = os.path.join(ROOT, rel)
        r = analyze(full)
        r["gate"] = name
        r["registered"] = not args.all or rel in {p for _, p in parse_registry(REGISTRY)}
        results.append(r)

    if args.gate:
        print(json.dumps(results[0], ensure_ascii=False, indent=2))
        return 0

    buckets = {"H": [], "M": [], "D": [], "?": []}
    for r in results:
        buckets.setdefault(r.get("tier", "?"), []).append(r)

    print(f"扫描门数：{len(results)}（注册清单 {len(parse_registry(REGISTRY))}）")
    for t in ("H", "M", "D"):
        print(f"  {t} 档：{len(buckets.get(t, []))}")
    print(f"  含「负控」字样的门：{sum(1 for r in results if r.get('negctl_mentions'))}")
    ss = [r for r in results if r.get("single_shot")]
    print(f"  ★ 单发取样（有负控 + 概率性现象 + 无多轮结构）：{len(ss)}")

    if args.json:
        with open(args.json, "w", encoding="utf-8") as f:
            json.dump(results, f, ensure_ascii=False, indent=2)
        print(f"JSON 已写：{args.json}")

    print("\n=== H 档（单次观测定生死，优先看） ===")
    for r in buckets.get("H", [])[: args.top]:
        ev = r["evidence"][0] if r["evidence"] else "-"
        print(f"  {os.path.basename(os.path.dirname(r['path'])):<34} 负控x{r['negctl_mentions']:<3} {ev}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
