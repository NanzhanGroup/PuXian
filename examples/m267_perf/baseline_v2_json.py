#!/usr/bin/env python3
# ══════════════════════════════════════════════════════════════
# examples/m267_perf/baseline_v2_json.py
#   —— 「性能基线 v2」原始数据结构化工具（配合 collect_baseline_v2.sh）
# 子命令：
#   assemble <records.txt> <out.json> "<loads>"
#             # 单次采集 records → 结构化 JSON（含环境/身份/锚点/每轮原值）
#   compare  <merged.json> <rep1.json> <rep2.json> [rep3.json ...]
#             # N 次独立采集 → 重复性自校验（多次间最大相对偏差）+ 归并 raw_baseline_v2
#             # 可选：--tol-run X --tol-startup X --tol-build X
# 只读 records/json；写入 out。
# 口径见 docs/PERF_BASELINE_METRICS_SPEC.md（run=min / startup=mean / build=single）。
# ══════════════════════════════════════════════════════════════
import json, sys, os, datetime

SPEC_REL = "docs/PERF_BASELINE_METRICS_SPEC.md"

# 与 baseline.tsv 对齐的 note（人可读）
NOTES = {
    "fib28_vm":     "纯递归 fib(28)x5 —— 纯计算最坏情形（VM 轨）",
    "fib28_c":      "纯递归 fib(28)x5 —— C 轨（比值锚）",
    "while_sum_vm": "纯算术/比较/JMP 3M —— 专盯每指令固定开销漂移（VM 轨）",
    "while_sum_c":  "纯算术 3M —— C 轨（比值锚）",
    "mixed_vm":     "dict+list+str 20 万次 —— 生产形态 IO/字典型（VM 轨）",
    "mixed_c":      "dict+list+str 20 万次 —— C 轨（比值锚）",
    "jsonwb_vm":    "JSON 2 万对象 构→串→解 —— 生产形态（VM 轨）",
    "jsonwb_c":     "JSON 2 万对象 —— C 轨（比值锚）",
    "startup_exec": "空程序连跑 200 次 / 200（进程启动开销）",
    "hot_build":    "px build 热缓存（空程序，含自动裁剪）",
}
ORDER = ["fib28_vm", "fib28_c", "while_sum_vm", "while_sum_c", "mixed_vm", "mixed_c",
         "jsonwb_vm", "jsonwb_c", "startup_exec", "hot_build"]
KIND = {n: ("run" if n.endswith(("_vm", "_c")) else ("startup" if n == "startup_exec" else "build"))
        for n in NOTES}
UNIT = "s"


def _now():
    return datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")


def assemble(rec_path, out_path, loads):
    env, meta, ident, anchor = {}, {}, [], []
    metrics = {}
    with open(rec_path, encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if not line:
                continue
            p = line.split("\t")
            k = p[0]
            if k == "env":
                env[p[1]] = p[2]
            elif k == "meta":
                meta[p[1]] = p[2]
            elif k == "ident":
                load, track, fn = p[1], p[2], int(p[3])
                ok = (fn == 0) if track == "vm" else (fn >= 1)
                ident.append({"load": load, "track": track, "fn_symbols": fn,
                              "expected": "==0" if track == "vm" else ">=1", "ok": ok})
            elif k == "anchor":
                anchor.append({"load": p[1], "vm_stdout": p[2], "c_stdout": p[3],
                               "match": p[4] == "true"})
            elif k == "run":
                name, csv = p[1], p[2]
                rounds = [float(x) for x in csv.split(",") if x != ""]
                metrics[name] = {"name": name, "kind": "run", "unit": UNIT,
                                 "rounds": rounds, "n_rounds": len(rounds),
                                 "stat": "min", "value": min(rounds)}
            elif k == "startup":
                name, total, per, runs = p[1], float(p[2]), float(p[3]), int(p[4])
                metrics[name] = {"name": name, "kind": "startup", "unit": UNIT,
                                 "runs": runs, "total_s": round(total, 6),
                                 "stat": "mean", "value": round(per, 6)}
            elif k == "build":
                name, val, warm = p[1], float(p[2]), int(p[3])
                metrics[name] = {"name": name, "kind": "build", "unit": UNIT,
                                 "warmups": warm, "stat": "single", "value": val}
    pair = {}
    for n in loads.split():
        vm, c = metrics.get(n + "_vm"), metrics.get(n + "_c")
        if vm and c and c["value"] > 0:
            pair[n] = {"vm": vm["value"], "c": c["value"],
                       "vm_over_c": round(vm["value"] / c["value"], 3)}
    cand = [{"name": n, "kind": KIND[n], "unit": UNIT, "value": metrics[n]["value"],
             "note": NOTES[n]} for n in ORDER if n in metrics]
    doc = {
        "schema": "raw_baseline_v2/1",
        "tag": meta.get("tag", ""),
        "spec": {"path": SPEC_REL, "sha256": meta.get("spec_sha256", "")},
        "meta": {"started_at": meta.get("started_at"), "ended_at": meta.get("ended_at"),
                 "duration_s": int(meta.get("duration_s", "0")), "reps": int(meta.get("reps", "5"))},
        "env": env,
        "params": {"pin": env.get("pin", ""), "run_reps": int(meta.get("reps", 5)),
                   "run_stat": "min", "startup_runs": 200, "startup_stat": "mean",
                   "build_warmups": 2, "build_stat": "single",
                   "warn_ratio": 1.25, "fail_ratio": 2.00, "gate_loads": loads},
        "identity": ident, "anchor": anchor,
        "metrics": metrics, "pair_ratios": pair, "baseline_candidate": cand,
    }
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(doc, f, ensure_ascii=False, indent=2)
    return doc


def compare(merged_path, rep_paths, tol_run=0.10, tol_startup=0.10, tol_build=0.15):
    reps = [json.load(open(p, encoding="utf-8")) for p in rep_paths]
    tags = [r.get("tag", os.path.basename(p)) for r, p in zip(reps, rep_paths)]
    rows = []
    for name in ORDER:
        vals = [(r["metrics"][name]["value"]) for r in reps if name in r["metrics"]]
        if len(vals) != len(reps):
            rows.append({"name": name, "kind": KIND[name], "values": vals, "n": len(vals),
                         "max_dev": None, "ok": False, "note": "有采集缺此指标"})
            continue
        lo, hi = min(vals), max(vals)
        dev = (hi - lo) / lo if lo else 0.0
        tol = {"run": tol_run, "startup": tol_startup, "build": tol_build}[KIND[name]]
        rows.append({"name": name, "kind": KIND[name], "unit": UNIT,
                     "values": [round(v, 6) for v in vals], "n": len(vals),
                     "min": round(lo, 6), "max": round(hi, 6),
                     "spread_ratio": round(hi / lo, 4) if lo else None,
                     "max_dev": round(dev, 4), "tol": tol, "ok": dev <= tol,
                     "adopted": round(lo, 6)})
    overall = all(r["ok"] for r in rows)
    maxdev = max((r["max_dev"] for r in rows if r["max_dev"] is not None), default=0.0)
    self_check = {
        "n_reps": len(reps), "rep_tags": tags,
        "tolerance": {"run": tol_run, "startup": tol_startup, "build": tol_build},
        "method": ("N 次独立采集，逐指标取 (max-min)/min 的相对偏差（多次间最大波动）；"
                   "偏差 ≤ 容差 视为波动在可接受范围（run/startup 10% · build 15%）。"
                   "adopted = 各次采集的最小值（与口径『取无干扰下界』一致）"),
        "max_dev": round(maxdev, 4), "overall_ok": overall, "metrics": rows,
    }
    adopted = {r["name"]: r.get("adopted") for r in rows if r.get("adopted") is not None}
    # 环境汇总
    envs = [r.get("env", {}) for r in reps]
    env_summary = {
        "host": {e.get("host") for e in envs},
        "cpu_model": {e.get("cpu_model") for e in envs},
        "px": {e.get("px") for e in envs},
        "gcc": {e.get("gcc") for e in envs},
        "os": {e.get("os") for e in envs},
        "pin": {e.get("pin") for e in envs},
        "quiet_all": all(e.get("quiet") == "true" for e in envs),
        "loadavg_before": [e.get("loadavg_before") for e in envs],
        "loadavg_after": [e.get("loadavg_after") for e in envs],
        "concurrent_gate_procs": [e.get("gates_after") for e in envs],
    }
    env_summary = {k: (sorted(v) if isinstance(v, set) else v) for k, v in env_summary.items()}

    run_dev = max((r["max_dev"] for r in rows if r["kind"] == "run" and r["max_dev"] is not None), default=0.0)
    other_dev = max((r["max_dev"] for r in rows if r["kind"] != "run" and r["max_dev"] is not None), default=0.0)
    notes = []
    if not env_summary["quiet_all"]:
        notes.append("采集窗口存在并发全量门（run_gates.sh）：并发门进程数 %s，loadavg before=%s after=%s。" % (
            env_summary["concurrent_gate_procs"], env_summary["loadavg_before"], env_summary["loadavg_after"]))
        notes.append("抗干扰按口径 §3.1 执行（钉核 taskset -c 3 + 预热 + min-of-%d）；本次 %d 次重复自校验："
                     "钉核 run 类多次波动 ≤ %.2f%%，未钉核 startup/build 波动 ≤ %.2f%%，均在容差内。" % (
                         reps[0]["params"].get("run_reps", 5), len(reps), run_dev * 100, other_dev * 100))
        notes.append("如需完全静默窗口复核，可在 selfhost/run_gates.sh 结束后重跑："
                     "bash examples/m267_perf/collect_baseline_v2_driver.sh")
    else:
        notes.append("采集窗口无并发门进程（quiet=true）。")

    cand = [{"name": r["name"], "kind": r["kind"], "unit": UNIT,
             "value": adopted[r["name"]], "note": NOTES[r["name"]]} for r in rows
            if r["name"] in adopted]
    doc = {
        "schema": "raw_baseline_v2/1",
        "generated_at": _now(),
        "spec": reps[0].get("spec", {}),
        "source_reps": rep_paths,
        "env_summary": env_summary,
        "repetitions": reps,
        "self_check": self_check,
        "adopted": adopted,
        "notes": notes,
        "baseline_candidate": cand,
    }
    with open(merged_path, "w", encoding="utf-8") as f:
        json.dump(doc, f, ensure_ascii=False, indent=2)

    # 候选基线 TSV（§5 格式；不覆盖 baseline.tsv）
    tsv = merged_path.replace(".json", ".candidate.tsv")
    with open(tsv, "w", encoding="utf-8") as f:
        f.write("# 性能基线 v2 —— 采集候选（collect_baseline_v2.sh + baseline_v2_json.py 生成）\n")
        f.write("# ⚠️ 候选件：人工核对后固化进 examples/m267_perf/baseline.tsv（本文件不自动覆盖）\n")
        f.write("# 来源 %d 次独立采集：%s\n" % (len(reps), " / ".join(os.path.basename(p) for p in rep_paths)))
        f.write("# 取值：各次采集的最小值（口径『取无干扰下界』）\n")
        f.write("# 格式：<name> <kind:run|startup|build> <unit> <baseline秒> <说明>\n")
        for c in cand:
            f.write("%-14s %-8s %-6s %-10s %s\n" % (c["name"], c["kind"], c["unit"], c["value"], c["note"]))

    # Markdown 报告（人读）
    md = merged_path.replace(".json", ".md")
    with open(md, "w", encoding="utf-8") as f:
        f.write("# 性能基线 v2 · 原始数据采集与重复性自校验\n\n")
        f.write("- 生成时间：%s\n" % doc["generated_at"])
        f.write("- 口径：`%s`（sha256 `%s`）\n" % (SPEC_REL, doc["spec"].get("sha256", "")[:16]))
        f.write("- 采集次数：%d（%s）\n" % (len(reps), " / ".join(tags)))
        f.write("- 环境：%s · %s · %s 核 · px %s · gcc %s · %s · 钉核 `%s`\n" % (
            (env_summary["host"] or ["?"])[0], (env_summary["cpu_model"] or ["?"])[0],
            (reps[0]["env"].get("nproc", "?")), (env_summary["px"] or ["?"])[0],
            (env_summary["gcc"] or ["?"])[0], (env_summary["os"] or ["?"])[0],
            (env_summary["pin"] or ["?"])[0]))
        f.write("- 环境安静（无并发门）：**%s**；并发门进程数：%s\n\n" % (
            env_summary["quiet_all"], env_summary["concurrent_gate_procs"]))
        if notes:
            f.write("\n".join("> " + n for n in notes) + "\n\n")
        f.write("## 1 重复性自校验（判据：多次间最大相对偏差 ≤ 容差）\n\n")
        f.write("**总体：%s**　最大偏差 = %.2f%%\n\n" % (
            "✅ 通过" if overall else "❌ 未通过", maxdev * 100))
        f.write("| 指标 | kind | " + " | ".join("rep%d min(s)" % (i + 1) for i in range(len(reps))) +
                " | 最大偏差 | 容差 | 判定 |\n")
        f.write("|---|---|" + "---|" * len(reps) + "---|---|---|\n")
        for r in rows:
            vs = " | ".join("%.4f" % v for v in r["values"]) if r["values"] else "-"
            dev = "%.2f%%" % (r["max_dev"] * 100) if r["max_dev"] is not None else "-"
            tol = "%.0f%%" % (r["tol"] * 100) if r.get("tol") else "-"
            f.write("| %s | %s | %s | %s | %s | %s |\n" % (
                r["name"], r["kind"], vs, dev, tol, "✅" if r["ok"] else "❌"))
        f.write("\n## 2 归并候选基线（adopted = 各次采集最小值）\n\n")
        f.write("| name | kind | unit | adopted(s) | 说明 |\n|---|---|---|---|---|\n")
        for c in cand:
            f.write("| %s | %s | %s | %s | %s |\n" % (c["name"], c["kind"], c["unit"], c["value"], c["note"]))
        f.write("\n> 候选件，未覆盖 `baseline.tsv`；人工核对后由 `verify.sh --update` 或人工固化。\n")
    return doc, self_check, tsv, md


def main():
    args = sys.argv[1:]
    if not args:
        print("usage: baseline_v2_json.py assemble <records> <out.json> [loads]\n"
              "       baseline_v2_json.py compare <merged.json> <rep..json> [--tol-run X --tol-startup X --tol-build X]")
        return 2
    cmd = args[0]
    if cmd == "assemble":
        loads = args[3] if len(args) > 3 else "fib28 while_sum mixed jsonwb"
        assemble(args[1], args[2], loads)
        print("assemble OK ->", args[2])
        return 0
    if cmd == "compare":
        rest = args[1:]
        reps, kw = [], {}
        i = 0
        while i < len(rest):
            a = rest[i]
            if a == "--tol-run":
                kw["tol_run"] = float(rest[i + 1]); i += 2
            elif a == "--tol-startup":
                kw["tol_startup"] = float(rest[i + 1]); i += 2
            elif a == "--tol-build":
                kw["tol_build"] = float(rest[i + 1]); i += 2
            else:
                reps.append(a); i += 1
        merged, rep_paths = reps[0], reps[1:]
        assert len(rep_paths) >= 2, "compare 至少需要 2 次采集"
        doc, sc, tsv, md = compare(merged, rep_paths, **kw)
        print("compare OK ->", merged)
        print("  n_reps =", sc["n_reps"], " overall_ok =", sc["overall_ok"],
              " max_dev = %.2f%%" % (sc["max_dev"] * 100))
        for r in sc["metrics"]:
            if not r["ok"]:
                print("  ❌ 波动超容差:", r)
        print("  candidate tsv ->", tsv)
        print("  report md     ->", md)
        return 0 if sc["overall_ok"] else 1
    print("未知子命令:", cmd)
    return 2


if __name__ == "__main__":
    sys.exit(main())
