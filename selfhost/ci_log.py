#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""ci_log.py —— 读 GitHub Actions 的 **run / job / job 日志**（M251s1 建立 · 缺陷 443）

【为什么有它 —— 一条代价很大的错误结论】
    我在 R63–R76 反复把「本仓 job 日志非管理员不可读（API 403）」当成事实，于是只用
    **step 结尾的注解**这个残缺通道排障：看不到「哪一步红、红在哪一行」，多轮
    「依赖真因定位不到」都源于此（M251 step7 就是靠注解只看到 5 行自证输出，
    而完整日志里**明明写着** `set: Illegal option -o pipefail`）。
    真相：本机一直有 `/data/pat.md`（token）与 `/data/fetchlog.py`（处理 302 → 签名 URL）
    ⇒ **日志一直可读**。本脚本把它固化进仓库，避免下一次再走同样的弯路。

【用法】
    ci_log.py runs [--limit N]              # 最近 N 个 run（按 sha 前缀筛用 --sha）
    ci_log.py jobs <run_id>                 # 该 run 的 job + **失败的 step（含名字）**
    ci_log.py log  <job_id> [--grep PAT] [--tail N] [--out FILE]

【token】$GH_TOKEN，或 $GH_TOKEN_FILE（默认 `/data/pat.md`）—— **绝不写入仓库**。
"""
import argparse
import json
import os
import re
import sys
import urllib.error
import urllib.request

REPO = os.environ.get("PX_REPO", "NanzhanGroup/PuXian")
API = "https://api.github.com"


def token():
    t = os.environ.get("GH_TOKEN", "").strip()
    if t:
        return t
    p = os.environ.get("GH_TOKEN_FILE", "/data/pat.md")
    try:
        return open(p).read().strip()
    except OSError:
        print("❌ 无 token：设 $GH_TOKEN，或让 $GH_TOKEN_FILE（默认 /data/pat.md）可读", file=sys.stderr)
        sys.exit(2)


def api(path, raw=False):
    req = urllib.request.Request(
        API + path,
        headers={"Authorization": "Bearer " + token(),
                 "Accept": "application/vnd.github+json",
                 "User-Agent": "puxian-ci-log",
                 "X-GitHub-Api-Version": "2022-11-28"},
    )
    try:
        with urllib.request.urlopen(req, timeout=60) as r:
            data = r.read()
            return data if raw else json.loads(data.decode("utf-8", "replace"))
    except urllib.error.HTTPError as e:
        msg = e.read().decode("utf-8", "replace")[:300]
        print("❌ HTTP %d %s\n   %s" % (e.code, path, msg), file=sys.stderr)
        if e.code == 403:
            print("   ℹ️ 403 多为**限流**或权限；本仓 job 日志**可读**（见本文件头注）。", file=sys.stderr)
        sys.exit(1)


def cmd_runs(a):
    d = api("/repos/%s/actions/runs?per_page=%d" % (REPO, a.limit))
    for r in d.get("workflow_runs", []):
        sha = r["head_sha"][:8]
        if a.sha and not r["head_sha"].startswith(a.sha):
            continue
        print("%-12s %-22s %-9s %s  %s" % (r["id"], r["name"], r["conclusion"] or r["status"], sha,
                                           r["display_title"][:60]))
        print("             %s" % r["html_url"])


def cmd_jobs(a):
    d = api("/repos/%s/actions/runs/%s/jobs?per_page=100" % (REPO, a.run_id))
    bad = 0
    for j in d.get("jobs", []):
        mark = "✅" if j["conclusion"] == "success" else ("⏭" if j["conclusion"] == "skipped" else "❌")
        print("%s %-12s %-10s %s" % (mark, j["id"], j["conclusion"] or j["status"], j["name"]))
        if j["conclusion"] not in ("success", "skipped", None):
            bad += 1
            for s in j.get("steps", []):
                if s.get("conclusion") not in ("success", "skipped", None):
                    print("      ⛔ step %s [%s] %s" % (s["number"], s["conclusion"], s["name"]))
    if bad:
        print("\n⇒ %d 个 job 红。取日志：ci_log.py log <job_id>" % bad)


def cmd_log(a):
    url = "/repos/%s/actions/jobs/%s/logs" % (REPO, a.job_id)


    class NoRedir(urllib.request.HTTPRedirectHandler):
        def redirect_request(self, *x):
            return None

    req = urllib.request.Request(
        API + url,
        headers={"Authorization": "Bearer " + token(), "User-Agent": "puxian-ci-log"},
    )
    loc = ""
    try:
        urllib.request.build_opener(NoRedir).open(req, timeout=60)
    except urllib.error.HTTPError as e:
        loc = (e.headers or {}).get("Location", "")
    if not loc:
        print("❌ 未拿到签名下载链接（job 可能未被保留）", file=sys.stderr)
        sys.exit(1)
    with urllib.request.urlopen(loc, timeout=300) as r:
        raw = r.read().decode("utf-8", "replace")
    txt = re.sub(r"\x1b\[[0-9;]*m", "", raw)          # 去 ANSI
    txt = re.sub(r"^\S*Z ", "", txt, flags=re.M)      # 去行首时间戳
    if a.out:
        open(a.out, "w", encoding="utf-8").write(txt)
        print("saved %d bytes -> %s（%d 行）" % (len(txt), a.out, txt.count("\n")), file=sys.stderr)
    lines = txt.splitlines()
    if a.grep:
        lines = [l for l in lines if re.search(a.grep, l)]
    if a.tail:
        lines = lines[-a.tail:]
    print("\n".join(lines))


def main():
    p = argparse.ArgumentParser(description="读 GitHub Actions run/job/日志")
    sub = p.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("runs"); r.add_argument("--limit", type=int, default=10); r.add_argument("--sha", default="")
    r.set_defaults(f=cmd_runs)
    j = sub.add_parser("jobs"); j.add_argument("run_id"); j.set_defaults(f=cmd_jobs)
    g = sub.add_parser("log"); g.add_argument("job_id"); g.add_argument("--out"); g.add_argument("--grep"); g.add_argument("--tail", type=int)
    g.set_defaults(f=cmd_log)
    a = p.parse_args()
    a.f(a)


if __name__ == "__main__":
    main()
