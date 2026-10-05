#!/usr/bin/env python3
# examples/m267_perf/freeze_baseline_v2.py
#   —— 「性能基线 v2」**固化**工具（任务 #166 / t3「基线固化和提交」）
#
# 作用（t2 采集 → **t3 固化** → t4/t5 判定，见 docs/PERF_BASELINE_METRICS_SPEC.md §11）：
#   把 t2 的 raw_baseline_v2.json（3 次独立采集的**全量原始轮次**）整理为**仓库约定的基线文件**：
#     ① examples/m267_perf/baseline.tsv            —— 门直接读取的「单一事实源」（口径 §5 格式）
#     ② examples/m267_perf/baseline_v2.json        —— 冻结的结构化基线（含指标 min/中位数/分位数、
#                                                       容差、采集时间、commit id 等元信息）
#     ③ examples/m267_perf/baseline_v2.md          —— 人读固化报告
#
# 取值口径：与 docs/PERF_BASELINE_METRICS_SPEC.md 一致 ——
#   · run   ：多次采集的**最小值**（§3.1「取无干扰下界」min，非均值/中位）
#   · startup：单次 = 空程序 200 次总墙钟 / 200（§3.2）；多次取 min
#   · build ：单次热构建墙钟（§3.3）；多次取 min
#   中位数/分位数**只作元信息留档**（供复算与波动审阅），**不用于门判据**。
#
# 只读 raw，只写上述 3 个产物；幂等可重跑（同 raw → 同 baseline.tsv 数值）。
import argparse
import datetime
import hashlib
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
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


def _sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def percentile(sorted_vals, q):
    """线性插值分位数（与 numpy.percentile 默认一致）。q ∈ [0,100]。"""
    if not sorted_vals:
        return None
    if len(sorted_vals) == 1:
        return sorted_vals[0]
    k = (len(sorted_vals) - 1) * (q / 100.0)
    lo = int(k)
    hi = min(lo + 1, len(sorted_vals) - 1)
    frac = k - lo
    return sorted_vals[lo] + (sorted_vals[hi] - sorted_vals[lo]) * frac


def pooled_rounds(rep, name):
    """取一个指标在某次采集里的**全部原始轮次**（run=各轮；startup/build=单次值）。"""
    m = rep["metrics"][name]
    if "rounds" in m:
        return [float(x) for x in m["rounds"]]
    return [float(m["value"])]


def git_commit():
    try:
        cid = subprocess.check_output(["git", "-C", ROOT, "rev-parse", "HEAD"], text=True).strip()
        subj = subprocess.check_output(["git", "-C", ROOT, "log", "-1", "--format=%s"], text=True).strip()
        return cid, subj
    except Exception:
        return "", ""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--raw", default=os.path.join(HERE, "raw_baseline_v2.json"))
    ap.add_argument("--out-json", default=os.path.join(HERE, "baseline_v2.json"))
    ap.add_argument("--out-md", default=os.path.join(HERE, "baseline_v2.md"))
    ap.add_argument("--baseline", default=os.path.join(HERE, "baseline.tsv"))
    ap.add_argument("--commit", default="", help="代码版本（缺省取 git HEAD）")
    args = ap.parse_args()

    raw = json.load(open(args.raw, encoding="utf-8"))
    reps = raw["repetitions"]
    params = reps[0].get("params", {})
    tol = raw.get("self_check", {}).get("tolerance", {"run": 0.10, "startup": 0.10, "build": 0.15})
    spec_sha = raw.get("spec", {}).get("sha256", "")
    if not spec_sha:
        spec_path = os.path.join(ROOT, SPEC_REL)
        spec_sha = _sha256(spec_path) if os.path.exists(spec_path) else ""

    # 采集时间窗口（3 次采集的起止）
    starts = [r["meta"]["started_at"] for r in reps if "meta" in r]
    ends = [r["meta"]["ended_at"] for r in reps if "meta" in r]
    collected_at = {"start": min(starts) if starts else "", "end": max(ends) if ends else ""}

    commit_id = args.commit
    commit_subj = ""
    if not commit_id:
        commit_id, commit_subj = git_commit()

    # ── 逐指标统计（adopted=min；中位/分位仅元信息）──
    metrics = []
    for name in ORDER:
        per_rep = [round(float(r["metrics"][name]["value"]), 6) for r in reps if name in r["metrics"]]
        pooled = []
        for r in reps:
            if name in r["metrics"]:
                pooled.extend(pooled_rounds(r, name))
        pooled_sorted = sorted(pooled)
        lo = min(per_rep) if per_rep else None
        metrics.append({
            "name": name,
            "kind": KIND[name],
            "unit": UNIT,
            "stat": "min",
            "adopted": round(lo, 6) if lo is not None else None,
            "min": round(min(pooled_sorted), 6) if pooled_sorted else None,
            "median": round(percentile(pooled_sorted, 50), 6) if pooled_sorted else None,
            "p25": round(percentile(pooled_sorted, 25), 6) if pooled_sorted else None,
            "p75": round(percentile(pooled_sorted, 75), 6) if pooled_sorted else None,
            "p95": round(percentile(pooled_sorted, 95), 6) if pooled_sorted else None,
            "n_values": len(pooled_sorted),
            "per_rep": per_rep,
            "rounds": [round(v, 6) for v in pooled],
            "note": NOTES[name],
        })

    # ── 基线表行（门读的 5 列）──
    tsv_rows = [{"name": m["name"], "kind": m["kind"], "unit": m["unit"],
                 "base": m["adopted"], "note": NOTES[m["name"]]}
                for m in metrics if m["adopted"] is not None]

    env = reps[0].get("env", {})
    doc = {
        "schema": "baseline_v2/1",
        "generated_at": _now(),
        "collected_at": collected_at,
        "source_commit": {"id": commit_id, "subject": commit_subj},
        "spec": {"path": SPEC_REL, "sha256": spec_sha},
        "source": {"raw": os.path.relpath(args.raw, ROOT), "n_reps": len(reps),
                   "rep_tags": [r.get("tag") for r in reps]},
        "env": env,
        "params": params,
        "tolerance": tol,
        "metrics": metrics,
        "baseline_tsv": tsv_rows,
        "identity": reps[0].get("identity", []),
        "anchor": reps[0].get("anchor", []),
        "self_check": raw.get("self_check", {}),
        "notes": raw.get("notes", []),
    }
    with open(args.out_json, "w", encoding="utf-8") as f:
        json.dump(doc, f, ensure_ascii=False, indent=2)

    # ── 固化 baseline.tsv（口径 §5 格式；含采集时间 / commit id 元信息）──
    px_ver = env.get("px", "")
    host = env.get("host", "")
    cpu = env.get("cpu_model", "")
    nproc = env.get("nproc", "")
    gcc = env.get("gcc", "")
    osv = env.get("os", "")
    with open(args.baseline, "w", encoding="utf-8") as f:
        f.write("# M267 性能回归基线（v2 固化） —— 由 examples/m267_perf/freeze_baseline_v2.py 生成\n")
        f.write("# 生成机：%s · %s · %s 核 · gcc %s · %s\n" % (host, cpu, nproc, gcc, osv))
        f.write("# 工具链：px %s\n" % px_ver)
        f.write("# 格式：<name> <kind:run|startup|build> <unit> <baseline秒> <说明>\n")
        f.write("# ⚠️ 绝对值只对**同机同工具链**有意义；判据是**相对变化**。刷新前须人工确认。\n")
        f.write("# ⚠️ 换机器 / 换 gcc / 换 px 版本 ⇒ 必须 --update 重定基（否则比值无意义）\n")
        f.write("# 依据：docs/PERF_BASELINE_V2.md（M266 建立）· docs/PERF_BASELINE_METRICS_SPEC.md（口径）\n")
        f.write("# 采集时间：%s .. %s（%d 次独立采集 · 固化值 = 各次最小值 min）\n"
                % (collected_at["start"], collected_at["end"], len(reps)))
        f.write("# 代码版本：%s（%s）\n" % (commit_id or "?", commit_subj or "?"))
        f.write("# 口径指纹：sha256 %s\n" % spec_sha)
        for r in tsv_rows:
            f.write("%-14s %-8s %-6s %-10s %s\n"
                    % (r["name"], r["kind"], r["unit"], r["base"], r["note"]))

    # ── 人读报告 ──
    with open(args.out_md, "w", encoding="utf-8") as f:
        f.write("# 性能基线 v2 · 固化报告\n\n")
        f.write("- 固化时间：%s\n" % doc["generated_at"])
        f.write("- 采集时间：%s .. %s（%d 次独立采集：%s）\n"
                % (collected_at["start"], collected_at["end"], len(reps), ", ".join(doc["source"]["rep_tags"])))
        f.write("- 代码版本：`%s`（%s）\n" % (commit_id or "?", commit_subj or "?"))
        f.write("- 口径：`%s`（sha256 `%s`）\n" % (SPEC_REL, spec_sha))
        f.write("- 环境：%s · %s · %s 核 · gcc %s · %s · px %s\n\n"
                % (host, cpu, nproc, gcc, osv, px_ver))
        f.write("## 固化值（门用 baseline.tsv；adopted = 各次最小值 min）\n\n")
        f.write("| name | kind | unit | adopted(s) | median(s) | p25 | p75 | p95 | 说明 |\n")
        f.write("|---|---|---|---|---|---|---|---|---|\n")
        for m in metrics:
            f.write("| %s | %s | %s | %s | %s | %s | %s | %s | %s |\n"
                    % (m["name"], m["kind"], m["unit"], m["adopted"], m["median"],
                       m["p25"], m["p75"], m["p95"], m["note"]))
        f.write("\n> run 门判据用 **min**（口径 §3.1）；median/p25/p75/p95 仅元信息留档，**不参与判红**。\n")
        f.write("> 容差：run %s / startup %s / build %s（多次采集间最大相对偏差上限）。\n"
                % (tol.get("run"), tol.get("startup"), tol.get("build")))
        f.write("> 门阈值：warn %s× / fail %s×（docs/PERF_BASELINE_METRICS_SPEC.md §6）。\n"
                % (params.get("warn_ratio", 1.25), params.get("fail_ratio", 2.0)))

    print("frozen  -> %s" % os.path.relpath(args.out_json, ROOT))
    print("report  -> %s" % os.path.relpath(args.out_md, ROOT))
    print("baseline-> %s" % os.path.relpath(args.baseline, ROOT))
    return 0


if __name__ == "__main__":
    sys.exit(main())
