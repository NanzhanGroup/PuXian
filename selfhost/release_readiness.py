#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# selfhost/release_readiness.py —— 发布就绪裁决（M323 · 缺陷 557）
# ------------------------------------------------------------
# 为什么需要（M322 第 5 轮实测，两次都是判据错）：
#   /tmp 里那版裁决器用**短 sha 字符串**做判据：
#       EXPECT_HEAD=9fb6c1cc
#       H=$(git rev-parse --short=8 HEAD)      # HEAD 19:31 后移动 ⇒ cd152820
#       [ "$H" = "$EXPECT_HEAD" ] || BAD=1     # ⇒ 判红
#   ① 假红：门覆盖的是 9fb6c1c，而 HEAD 只是多了一个只改 CHANGELOG 的提交
#      （19:39 用户令：CHANGELOG 不是代码，不该为它跑全量门）⇒ sha 文本必然不等，
#      但「待发布变更面 ⊆ 门覆盖变更面」这个契约成立 ⇒ 该判绿。
#   ② 假绿（更危险）：若它判绿，它会 `git tag -a v0.2.322 ... 9fb6c1c`，
#      把 tag 打在**旧提交**上，而 push 推的是 main ⇒ tag 与 main 脱节。
#      那次误判是**误打误撞**挡住了这个错。
#   ⇒ 判据必须贴契约，不贴表象（CRITERIA ①）；短 sha 是可变宽度的表象。
#     同样地：门数量不得硬编码（CRITERIA ②，绝对数不是判据）——
#     本脚本从注册源动态取，并用「日志自述的门清单数」三点交叉。
#
# 判据（契约式 · 每条都给证据）
#   R1 base 是 HEAD 的祖先（且 base 在仓库里可解析）
#   R2 base..HEAD 的变更路径 ⊆ 白名单（默认 CHANGELOG.md + 顶层 *.md + docs/）
#   R3 产品面/门面/工具面零变更（runtime/ stdlib/ tools/ spec/ selfhost/
#      packaging/ examples/ cmd/ internal/）—— R2 的反面，单列便于读因
#   R4 唯一读 CHANGELOG 的元门 check_defect_refs.py 通过（GATE_SCOPE 分档的对口证据）
#   R0 门日志可解析 —— **两种权威档都要认**（缺陷 558）：
#      · 分片档：shard1..4 + quiet（/data/gatevm/shard_run.sh）
#      · 串行权威档：「══ 汇总：失败 N 项」+ 逐门 ✅/❌ 行（selfhost/run_gates.sh 全量档）
#      R0 同时自检两档的**内部自洽**：汇总的失败数必须等于逐门红数，
#      否则一律判红（M323 第 6 轮实测：串行档日志被判「汇总段缺失」⇒ 假红 R0）
#   R5 门日志：各分片 rc=0、红 0、合计绿 == 注册门数 == 日志自述门清单数
#   R6 工作树干净（tag 要打在一个已落仓的提交上）
#   R7 tag 目标 == HEAD（禁止「tag 打在门覆盖的旧提交上」）
#
# 负控（--self-test · 每个场景先在临时仓库造出来，并自证扰动真的发生）
#   N1 产品面混入      ⇒ R3 红（前提：diff 里确实出现该文件）
#   N2 base 非祖先     ⇒ R1 红（前提：merge-base --is-ancestor 非 0）
#   N3 门日志红 1      ⇒ R5 红（前提：日志里确实是「红 1」）
#   N4 门数不符        ⇒ R5 红（前提：两数字确实不等）
#   N5 脏树            ⇒ R6 红（前提：porcelain 非空）
#   N6 正控：全绿      ⇒ 判绿（证明判据不是恒红）
#   N7 元门失败        ⇒ R4 红（前提：桩脚本 rc=1）
#   N9 串行权威档全绿  ⇒ GREEN（前提：解析出 mode=serial 且绿=3）
#   N10 串行档红 1     ⇒ R5 红（前提：逐门红=1 == 汇总「失败 1」）
#   N11 串行档自相矛盾 ⇒ R0 红（前提：汇总「失败 2」而逐门红=0）
#   任一负控未红 ⇒ 打印「负控失效」并 exit 3 —— 判据没牙就是没判据（CRITERIA ③）
#
# 用法
#   selfhost/release_readiness.py --gates-log <log> [--base <sha>] [--json]
#   selfhost/release_readiness.py --self-test
#   selfhost/release_readiness.py --gates-log <log> --notify     # 绿 p0 / 红 p1
#   selfhost/release_readiness.py --gates-log <log> --emit-tag-cmd
#   退出码：0 绿 · 1 红 · 2 参数/环境错 · 3 负控失效
# ============================================================
import argparse
import json
import os
import re
import subprocess
import sys
import tempfile

# ── 变更面（与 GATE_SCOPE 分档同构）──────────────────────────
PRODUCT_DIRS = ("runtime/", "stdlib/", "tools/", "spec/")
GATE_DIRS = ("selfhost/", "packaging/")
OTHER_CODE_DIRS = ("examples/", "cmd/", "internal/", "native/", "bootstrap/")
CODE_DIRS = PRODUCT_DIRS + GATE_DIRS + OTHER_CODE_DIRS
# 文档面：CHANGELOG 与其它说明文字 —— 运行时不读一行
DOC_ALLOW = re.compile(r"^(CHANGELOG\.md|[^/]+\.md|docs/[^/].*)$")


def sh(args, cwd=None, timeout=120):
    """跑一条命令，返回 (rc, stdout, stderr)。永不抛异常 —— 判据要能读到失败本身。"""
    try:
        p = subprocess.run(args, cwd=cwd, timeout=timeout,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return p.returncode, p.stdout.decode("utf-8", "replace"), p.stderr.decode("utf-8", "replace")
    except Exception as e:                                   # noqa: BLE001
        return 127, "", "%s" % e


class Verdict:
    """判据收集器：每条判据带 id / 契约 / 证据 / 结论。"""

    def __init__(self):
        self.items = []

    def add(self, cid, contract, ok, evidence):
        self.items.append({"id": cid, "contract": contract,
                           "ok": bool(ok), "evidence": evidence})

    @property
    def green(self):
        return all(i["ok"] for i in self.items)

    @property
    def failed(self):
        return [i for i in self.items if not i["ok"]]

    def as_dict(self):
        return {"verdict": "GREEN" if self.green else "RED",
                "checks": self.items,
                "failed_ids": [i["id"] for i in self.failed]}

    def render(self):
        out = []
        for i in self.items:
            mark = "✅" if i["ok"] else "❌"
            out.append("%s %-3s %s" % (mark, i["id"], i["contract"]))
            for line in i["evidence"].strip().splitlines():
                out.append("       %s" % line)
        out.append("")
        out.append("VERDICT=%s%s" % ("GREEN" if self.green else "RED",
                                     "" if self.green else " · 失败判据 " + ",".join(
                                         i["id"] for i in self.failed)))
        return "\n".join(out)


def registry_count(repo, registry_rel="selfhost/gates.registry.sh"):
    """从**单一注册源**数门的条数（不硬编码 229）。取 HEAD 版本，避免脏树干扰。"""
    rc, out, _ = sh(["git", "-C", repo, "show", "HEAD:%s" % registry_rel])
    if rc != 0:
        path = os.path.join(repo, registry_rel)
        if not os.path.exists(path):
            return None
        out = open(path, encoding="utf-8", errors="replace").read()
    return len(re.findall(r"(?m)^\s*run\s+\S+", out))


def parse_gates_log(text):
    """解析全量门日志的汇总段。返回 dict 或 {"error": ...}。"""
    shard = {}
    for m in re.finditer(r"(?m)^\s*(shard\d+|quiet)\s*:\s*绿\s*(\d+)\s*·\s*红\s*(\d+)"
                         r"(?:[^\n]*?rc=(\d+))?", text):
        shard[m.group(1)] = {"green": int(m.group(2)), "red": int(m.group(3)),
                             "rc": None if m.group(4) is None else int(m.group(4))}
    tot = None
    m = re.search(r"合计：绿\s*(\d+)\s*·\s*红\s*(\d+)(?:（门清单\s*(\d+)）)?", text)
    if m:
        tot = {"green": int(m.group(1)), "red": int(m.group(2)),
               "declared_total": None if m.group(3) is None else int(m.group(3))}
    base = None
    mb = re.search(r"建 worktree wt\d+ @ ([0-9a-f]{7,40})", text)
    if mb:
        base = mb.group(1)
    if base is None:
        # 串行权威档头行：=== M323 全量门（第 6 轮 · base=<sha>）· <ts> ===
        mh = re.search(r"base=([0-9a-f]{7,40})", text)
        if mh:
            base = mh.group(1)

    mode = "sharded"
    if not shard or tot is None:
        # ── 串行权威档（缺陷 558）：run_gates.sh 全量档只打「汇总：失败 N 项」，
        #    不给绿数 ⇒ 绿/红由**逐门状态行**数出来，并用汇总失败数做自洽校验。
        ms = re.search(r"汇总：失败\s*(\d+)\s*项(?:[^\n]*?跳过\s*(\d+)\s*门)?", text)
        if not ms:
            return {"error": "汇总段缺失（分片档「shardN: 绿 X · 红 Y / 合计」与"
                             "串行档「汇总：失败 N 项」都没有）",
                    "shards": shard, "total": tot, "base_from_log": base}
        skipped = int(ms.group(2) or 0)
        ok_n = len(re.findall(r"(?m)^✅\s+[A-Za-z0-9_.-]+（", text))
        bad_n = len(re.findall(r"(?m)^(?:❌|⏱)\s+[A-Za-z0-9_.-]+（", text))
        failed_decl = int(ms.group(1))
        if skipped:
            return {"error": "串行档跳过了 %d 门（--fail-fast/中途退出）⇒ 不是全量，不能裁决" % skipped,
                    "shards": shard, "total": tot, "base_from_log": base}
        if ok_n + bad_n == 0:
            return {"error": "串行档「汇总：失败 %d 项」在，但逐门状态行为 0（日志截断？）" % failed_decl,
                    "shards": shard, "total": tot, "base_from_log": base}
        if failed_decl != bad_n:
            return {"error": "串行档自相矛盾：汇总「失败 %d 项」≠ 逐门红 %d 条" % (failed_decl, bad_n),
                    "shards": shard, "total": tot, "base_from_log": base}
        rc_m = re.search(r"(?m)^rc=(\d+)\s*$", text)
        shard = {"serial": {"green": ok_n, "red": bad_n,
                            "rc": int(rc_m.group(1)) if rc_m else None}}
        tot = {"green": ok_n, "red": bad_n, "declared_total": None}
        mode = "serial"
    return {"shards": shard, "total": tot, "base_from_log": base, "mode": mode}


def evaluate(repo, gates_log, base=None, expect_head=None, defect_refs=None,
             registry_rel="selfhost/gates.registry.sh", log_text=None):
    """按 R1–R7 判一次发布是否就绪。纯函数式：所有输入显式，便于负控造场景。"""
    v = Verdict()

    # ── R0 前置：能读日志 ────────────────────────────────
    if log_text is None:
        if not gates_log or not os.path.exists(gates_log):
            v.add("R0", "门日志存在且可读", False, "日志不存在：%s" % gates_log)
            return v
        log_text = open(gates_log, encoding="utf-8", errors="replace").read()
    parsed = parse_gates_log(log_text)
    if "error" in parsed:
        v.add("R0", "门日志含可解析的汇总段", False, parsed["error"])
        return v
    v.add("R0", "门日志含可解析的汇总段", True,
          "%s · %d 组：绿 %d · 红 %d" % (
              "串行权威档" if parsed.get("mode") == "serial" else "分片档",
              len(parsed["shards"]), parsed["total"]["green"], parsed["total"]["red"]))

    # ── 解析提交 ────────────────────────────────────────
    head_full = sh(["git", "-C", repo, "rev-parse", "HEAD"])[1].strip()
    base_in = base or parsed.get("base_from_log")
    v.add("R1a", "门覆盖的基线可确定（显式 --base 或日志内 worktree 标注）",
          bool(base_in), "base=%s（来源：%s）" % (base_in, "--base" if base else "日志"))
    if not base_in:
        return v
    rc, base_full, err = sh(["git", "-C", repo, "rev-parse", base_in])
    base_full = base_full.strip()
    v.add("R1b", "基线在仓库里可解析", rc == 0, "base=%s rc=%s %s" % (base_full or base_in, rc, err.strip()))
    if rc != 0:
        return v

    # ── R1 祖先关系（契约：门的结论要能覆盖 HEAD 的代码面）───
    rc_anc, _, _ = sh(["git", "-C", repo, "merge-base", "--is-ancestor", base_full, head_full])
    v.add("R1", "基线是 HEAD 的祖先（否则门的结论覆盖不到 HEAD）",
          rc_anc == 0, "merge-base --is-ancestor %s %s ⇒ rc=%d" % (base_full[:8], head_full[:8], rc_anc))

    # ── R2/R3 变更面 ───────────────────────────────────
    rc, out, _ = sh(["git", "-C", repo, "diff", "--name-only", base_full, head_full])
    changed = [x.strip() for x in out.splitlines() if x.strip()]
    outside = [p for p in changed if not DOC_ALLOW.match(p)]
    code_touch = [p for p in changed if p.startswith(CODE_DIRS)]
    v.add("R2", "基线→HEAD 的变更路径全部落在文档白名单（CHANGELOG/顶层 md/docs/）",
          not outside, "变更 %d 个文件%s" % (len(changed),
                                          "" if not outside else "；越界：" + ", ".join(outside[:8])))
    v.add("R3", "产品面/门面/工具面零变更（runtime stdlib tools spec selfhost packaging 等）",
          not code_touch, "代码面变更 %d 个%s" % (len(code_touch),
                                              "" if not code_touch else "：" + ", ".join(code_touch[:8])))

    # ── R4 唯一读 CHANGELOG 的元门 ──────────────────────
    if defect_refs and os.path.exists(defect_refs):
        rc4, out4, err4 = sh([sys.executable, defect_refs], cwd=repo, timeout=600)
        tail = (out4 or err4).strip().splitlines()[-3:]
        v.add("R4", "check_defect_refs（读 CHANGELOG 的元门）通过 —— GATE_SCOPE 分档的对口证据",
              rc4 == 0, "rc=%d · %s" % (rc4, " / ".join(tail)))
    else:
        v.add("R4", "check_defect_refs 可执行", False,
              "未找到 %s（不允许跳过：CHANGELOG-only 变更的正当性全押在这条）" % defect_refs)

    # ── R5 门日志 ──────────────────────────────────────
    n_reg = registry_count(repo, registry_rel)
    tot = parsed["total"]
    reds = [k for k, x in parsed["shards"].items() if x["red"] != 0]
    rcs = [k for k, x in parsed["shards"].items() if x["rc"] not in (None, 0)]
    reasons = []
    if tot["red"] != 0:
        reasons.append("合计红 %d" % tot["red"])
    if reds:
        reasons.append("分片红：" + ",".join(sorted(reds)))
    if rcs:
        reasons.append("分片 rc≠0：" + ",".join(sorted(rcs)))
    if n_reg is None:
        reasons.append("注册源不可读，门数无法核对")
    else:
        if tot["green"] != n_reg:
            reasons.append("合计绿 %d ≠ 注册门数 %d" % (tot["green"], n_reg))
        if tot["declared_total"] is not None and tot["declared_total"] != n_reg:
            reasons.append("日志自述门清单 %d ≠ 注册门数 %d" % (tot["declared_total"], n_reg))
    v.add("R5", "门全绿且门数与注册源一致（数量动态取，不硬编码）",
          not reasons,
          "合计绿 %d · 红 %d · 注册 %s · 分片 %s%s" % (
              tot["green"], tot["red"], n_reg,
              ",".join("%s(%d/%d%s)" % (k, parsed["shards"][k]["green"], parsed["shards"][k]["red"],
                                        "" if parsed["shards"][k]["rc"] in (None, 0) else " rc=%d" % parsed["shards"][k]["rc"])
                       for k in sorted(parsed["shards"])),
              "" if not reasons else "；" + "；".join(reasons)))

    # ── R6 工作树 ──────────────────────────────────────
    rc6, out6, _ = sh(["git", "-C", repo, "status", "--porcelain"])
    dirt = [x for x in out6.splitlines() if x.strip()]
    v.add("R6", "工作树干净（tag 要打在一个已落仓的提交上）",
          not dirt, "porcelain %d 行%s" % (len(dirt), "" if not dirt else "：" + " | ".join(dirt[:5])))

    # ── R7 tag 目标 == HEAD ────────────────────────────
    target = expect_head or "HEAD"
    rc7, tgt_full, _ = sh(["git", "-C", repo, "rev-parse", target])
    tgt_full = tgt_full.strip()
    v.add("R7", "tag 目标就是 HEAD（禁止把 tag 打在门覆盖的旧提交上 —— M322 假绿场景）",
          tgt_full == head_full and bool(head_full),
          "目标 %s(%s) vs HEAD %s" % (target, tgt_full[:8], head_full[:8]))
    return v


# ───────────────────────── 负控自测 ─────────────────────────
def _mk_repo(tmp):
    """造一个最小仓库：一个基线提交 + 一个只改 CHANGELOG 的提交。"""
    repo = os.path.join(tmp, "repo")
    os.makedirs(repo)
    g = lambda *a: sh(["git", "-C", repo] + list(a))          # noqa: E731
    g("init", "-q")
    g("config", "user.email", "t@example.com")
    g("config", "user.name", "t")
    os.makedirs(os.path.join(repo, "runtime"), exist_ok=True)
    os.makedirs(os.path.join(repo, "selfhost"), exist_ok=True)
    open(os.path.join(repo, "runtime", "a.c"), "w").write("int a(void){return 0;}\n")
    open(os.path.join(repo, "selfhost", "gates.registry.sh"), "w").write(
        'run gate_a bash a.sh\nrun gate_b bash b.sh\nrun gate_c bash b.sh\n')
    open(os.path.join(repo, "CHANGELOG.md"), "w").write("# CHANGELOG\n")
    g("add", "-A")
    g("commit", "-q", "-m", "base")
    base = g("rev-parse", "HEAD")[1].strip()
    open(os.path.join(repo, "CHANGELOG.md"), "a").write("## M323 记录\n")
    g("add", "-A")
    g("commit", "-q", "-m", "doc only")
    return repo, base


def _log(shards, green, red, declared):
    lines = ["═══ 分片汇总 ═══"]
    for name, (g, r, rc) in shards.items():
        if name == "quiet":
            lines.append("  quiet : 绿 %d · 红 %d（低负载档）" % (g, r))
        else:
            lines.append("  %-6s: 绿 %d · 红 %d · rc=%d" % (name, g, r, rc))
    tail = "  合计：绿 %d · 红 %d" % (green, red)
    if declared is not None:
        tail += "（门清单 %d）" % declared
    lines.append(tail)
    return "\n".join(lines) + "\n"


def _serial_log(ok_names, bad_names, failed_decl=None, skipped=0):
    """造一份**串行权威档**日志（run_gates.sh 全量档的原样格式，缺陷 558）。"""
    lines = ["=== M323 全量门（第 7 轮 · base=deadbeefcafe）· 2026-10-23T04:00:00+08:00 ===", ""]
    for n in ok_names:
        lines.append("✅ %s（1s）" % n)
    for n in bad_names:
        lines.append("❌ %s（rc=1, 2s）" % n)
    fd = len(bad_names) if failed_decl is None else failed_decl
    tail = "══ 汇总：失败 %d 项" % fd
    if skipped:
        tail += " · 跳过 %d 门" % skipped
    tail += "（总耗时 3s）══"
    lines.append(tail)
    lines.append("rc=%d" % (1 if bad_names else 0))
    return "\n".join(lines) + "\n"


def self_test():
    ok = 0
    bad = 0
    dead = 0
    stub_ok = None

    def report(tag, passed, detail=""):
        nonlocal ok, bad
        if passed:
            ok += 1
            print("  ✅ %s" % tag)
        else:
            bad += 1
            print("  ❌ %s %s" % (tag, detail))

    with tempfile.TemporaryDirectory() as tmp:
        repo, base = _mk_repo(tmp)
        # 桩：check_defect_refs（可切成功/失败），用于 R4 的两侧
        stub = os.path.join(tmp, "stub_refs.py")
        open(stub, "w").write("import sys\nsys.exit(int(open(%r).read().strip()))\n"
                              % os.path.join(tmp, "stub.rc"))
        open(os.path.join(tmp, "stub.rc"), "w").write("0")
        log_green = _log({"shard1": (2, 0, 0), "shard2": (1, 0, 0)}, 3, 0, 3)

        print("===== 负控与正控（判据 R1–R7）=====")

        # N6 正控：全绿 ⇒ 必须判绿
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_green)
        report("N6 正控：文档-only + 门全绿 ⇒ GREEN", v.green,
               "失败判据 %s" % v.as_dict()["failed_ids"])

        # N1 产品面混入 ⇒ R3 红（前提自证：diff 里确实出现该文件）
        open(os.path.join(repo, "runtime", "b.c"), "w").write("int b(void){return 1;}\n")
        sh(["git", "-C", repo, "add", "-A"])
        sh(["git", "-C", repo, "commit", "-q", "-m", "touch runtime"])
        changed = sh(["git", "-C", repo, "diff", "--name-only", base, "HEAD"])[1]
        premise = "runtime/b.c" in changed
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_green)
        if not premise:
            dead += 1
            print("  ❌ N1 前提自证失败：扰动未发生（diff 无 runtime/b.c）")
        else:
            report("N1 产品面混入 ⇒ R3 判红（前提：diff 确有 runtime/b.c）",
                   (not v.green) and "R3" in v.as_dict()["failed_ids"])
        # 复原（让后续场景从干净文档向提交出发）
        sh(["git", "-C", repo, "reset", "-q", "--hard", "HEAD~1"])

        # N2 base 非祖先 ⇒ R1 红（前提：is-ancestor 非 0）
        # ⚠️ 场景要点：HEAD 必须停在**另一个提交**上，否则 base==HEAD 会自证成立
        #    （自测第一版就是这样假绿的：HEAD 恰在 side 上 ⇒ is-ancestor(side,side)=0）
        sh(["git", "-C", repo, "checkout", "-q", "-b", "side"])
        open(os.path.join(repo, "CHANGELOG.md"), "a").write("side\n")
        sh(["git", "-C", repo, "add", "-A"])
        sh(["git", "-C", repo, "commit", "-q", "-m", "side"])
        side = sh(["git", "-C", repo, "rev-parse", "HEAD"])[1].strip()
        sh(["git", "-C", repo, "checkout", "-q", "-"])       # HEAD 回到 side 的父提交
        head_now = sh(["git", "-C", repo, "rev-parse", "HEAD"])[1].strip()
        rc_anc, _, _ = sh(["git", "-C", repo, "merge-base", "--is-ancestor", side, head_now])
        v = evaluate(repo, None, base=side, defect_refs=stub, log_text=log_green)
        if rc_anc == 0 or side == head_now:
            dead += 1
            print("  ❌ N2 前提自证失败：side=%s head=%s rc=%d（side 竟仍是祖先）"
                  % (side[:8], head_now[:8], rc_anc))
        else:
            report("N2 基线非 HEAD 祖先 ⇒ R1 判红（前提：is-ancestor rc≠0 · side≠HEAD）",
                   "R1" in v.as_dict()["failed_ids"])

        # N3 门日志有红 ⇒ R5 红（前提：日志里确实是「红 1」）
        log_red = _log({"shard1": (2, 0, 0), "shard2": (0, 1, 0)}, 2, 1, 3)
        premise = parse_gates_log(log_red)["total"]["red"] == 1
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_red)
        if not premise:
            dead += 1
            print("  ❌ N3 前提自证失败：解析出的红不是 1")
        else:
            report("N3 门日志 红 1 ⇒ R5 判红（前提：解析得到红 1）",
                   "R5" in v.as_dict()["failed_ids"])

        # N4 门数不符 ⇒ R5 红（前提：两数字确实不等）
        log_short = _log({"shard1": (2, 0, 0)}, 2, 0, 2)
        n_reg = registry_count(repo)
        premise = n_reg == 3 and parse_gates_log(log_short)["total"]["green"] == 2
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_short)
        if not premise:
            dead += 1
            print("  ❌ N4 前提自证失败：注册门数=%s / 日志绿数=%s" % (
                n_reg, parse_gates_log(log_short)["total"]["green"]))
        else:
            report("N4 门数与注册源不符（2 vs 3）⇒ R5 判红（前提：两数字不等）",
                   "R5" in v.as_dict()["failed_ids"])

        # N5 脏树 ⇒ R6 红（前提：porcelain 非空）
        open(os.path.join(repo, "CHANGELOG.md"), "a").write("dirty\n")
        premise = bool(sh(["git", "-C", repo, "status", "--porcelain"])[1].strip())
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_green)
        if not premise:
            dead += 1
            print("  ❌ N5 前提自证失败：工作树竟然干净")
        else:
            report("N5 脏工作树 ⇒ R6 判红（前提：porcelain 非空）",
                   "R6" in v.as_dict()["failed_ids"])
        sh(["git", "-C", repo, "checkout", "-q", "--", "CHANGELOG.md"])

        # N7 元门失败 ⇒ R4 红（前提：桩 rc=1）
        open(os.path.join(tmp, "stub.rc"), "w").write("1")
        rc_stub = sh([sys.executable, stub])[0]
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_green)
        if rc_stub != 1:
            dead += 1
            print("  ❌ N7 前提自证失败：桩 rc=%d，期望 1" % rc_stub)
        else:
            report("N7 check_defect_refs 失败 ⇒ R4 判红（前提：桩 rc=1）",
                   "R4" in v.as_dict()["failed_ids"])

        # R7 反证：tag 目标指到基线（M322 假绿场景）⇒ 必须红
        open(os.path.join(tmp, "stub.rc"), "w").write("0")
        v_r7 = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_green,
                        expect_head=base)
        report("N8 tag 目标=基线（旧提交）⇒ R7 判红（M322 假绿场景）",
               "R7" in v_r7.as_dict()["failed_ids"])

        # ── N9–N11：串行权威档（缺陷 558：R0 曾把串行档日志判成「汇总段缺失」）──
        log_serial_green = _serial_log(["gate_a", "gate_b", "gate_c"], [])
        pser = parse_gates_log(log_serial_green)
        premise = pser.get("mode") == "serial" and pser["total"]["green"] == 3
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_serial_green)
        if not premise:
            dead += 1
            print("  ❌ N9 前提自证失败：mode=%s green=%s"
                  % (pser.get("mode"), pser.get("total", {}).get("green")))
        else:
            report("N9 串行权威档全绿 ⇒ GREEN（前提：mode=serial 且绿=3）",
                   v.green, "失败判据 %s" % v.as_dict()["failed_ids"])

        log_serial_red = _serial_log(["gate_a", "gate_b"], ["gate_c"])
        pser = parse_gates_log(log_serial_red)
        premise = pser.get("total", {}).get("red") == 1
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_serial_red)
        if not premise:
            dead += 1
            print("  ❌ N10 前提自证失败：解析出的逐门红不是 1")
        else:
            report("N10 串行档红 1 ⇒ R5 判红（前提：汇总「失败 1」== 逐门红 1）",
                   "R5" in v.as_dict()["failed_ids"])

        log_serial_lie = _serial_log(["gate_a", "gate_b"], [], failed_decl=2)
        perr = parse_gates_log(log_serial_lie)
        premise = "error" in perr and "自相矛盾" in perr["error"]
        v = evaluate(repo, None, base=base, defect_refs=stub, log_text=log_serial_lie)
        if not premise:
            dead += 1
            print("  ❌ N11 前提自证失败：汇总与逐门红的不一致未被识破")
        else:
            report("N11 串行档自相矛盾（汇总失败 2 · 逐门红 0）⇒ R0 判红",
                   "R0" in v.as_dict()["failed_ids"])

    print("===== 通过 %d 项 · 失败 %d 项 · 负控失效 %d 项 =====" % (ok, bad, dead))
    if dead:
        print("RELEASE-READINESS-SELFTEST-NEGCONTROL-DEAD")
        return 3
    if bad:
        print("RELEASE-READINESS-SELFTEST-FAIL")
        return 1
    print("RELEASE-READINESS-SELFTEST-OK")
    return 0


# ───────────────────────── 通知（NO_SELFCHANNEL：只走 dy-notify.sh）─────
def notify(green, milestone, summary):
    """绿 = 结论（用户可见 p0）；红 = 过程态（p1 只记账 + 自驱，不推用户）。"""
    exe = "/usr/local/sbin/dy-notify.sh"
    if not os.path.exists(exe):
        return "skip: dy-notify.sh 不存在"
    if green:
        rc, _, err = sh([exe, "p0", "M%s 发布就绪" % milestone, summary], timeout=60)
    else:
        rc, _, err = sh([exe, "p1", "release-m%s-red" % milestone,
                         "M%s 发布裁决判红" % milestone, summary], timeout=60)
    return "dy-notify rc=%d %s" % (rc, err.strip())


def main():
    ap = argparse.ArgumentParser(description="发布就绪裁决（M323）")
    ap.add_argument("--repo", default=".")
    ap.add_argument("--gates-log")
    ap.add_argument("--base")
    ap.add_argument("--at", default="HEAD", help="tag 目标（默认 HEAD；非 HEAD 一律判红）")
    ap.add_argument("--defect-refs", default="selfhost/check_defect_refs.py")
    ap.add_argument("--registry", default="selfhost/gates.registry.sh")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--notify", action="store_true")
    ap.add_argument("--milestone", default="?")
    ap.add_argument("--emit-tag-cmd", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args()

    if a.self_test:
        return self_test()

    repo = os.path.abspath(a.repo)
    v = evaluate(repo, a.gates_log, base=a.base, expect_head=a.at,
                 defect_refs=os.path.join(repo, a.defect_refs), registry_rel=a.registry)
    if a.json:
        print(json.dumps(v.as_dict(), ensure_ascii=False, indent=2))
    else:
        print(v.render())

    if a.notify:
        print(notify(v.green, a.milestone, v.render().splitlines()[-1]))

    if v.green and a.emit_tag_cmd:
        print("\n# 判绿 ⇒ 唯一合法创建入口（main + tag 同一次 push）")
        print("packaging/make_tag.sh --milestone %s --push" % a.milestone)
    return 0 if v.green else 1


if __name__ == "__main__":
    sys.exit(main())
