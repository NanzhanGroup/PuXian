#!/usr/bin/env bash
# ============================================================
# packaging/chain/ci_parity_check.sh —— **远端 CI 与本地裁决的对账器**（M415 · 缺陷 826）
# ------------------------------------------------------------
# 为什么有它（2026-11-18 04:2x 实测）：
#   GitHub 上 PuXian CI **连续 15 轮红**（最早可查到 11-14），本地全量门 289/289 绿，
#   发布链照常判 GREEN 并打 tag —— **两条判据给出相反结论，没有任何人知道**。
#   「本地绿」只证明这台机器；只要不说清楚这一点，绿就是局部真理冒充全局真理。
#
# 契约（口径贴契约，不贴表象）：
#   0  OK        —— 远端 CI 绿（没有可对账的红）
#   0  SYNC      —— 远端 CI 红，但每个红门都在 ci_known_red.tsv 登记且未过期
#   1  GAP       —— 出现**未登记**的红门，或登记已过期（= 红在漂，没人负责）
#   2  ENV-FAIL  —— 取不到数（令牌失效 / 网络 / 无 fixture）⇒ 响亮失败，**不许当空结果**
#
# 数据来源：packaging/gh_get.sh（唯一实现，401/降级/失败语义都由它承担）；
#   自测用 --fixture <dir>（内含 runs.json + jobs.json），不碰网络。
# 台账：<门名>\t<定性>\t<到期 YYYY-MM-DD>（到期过了 ⇒ GAP）
#
# 用法：
#   ci_parity_check.sh [--fixture DIR] [--known FILE] [--quiet]
#   ci_parity_check.sh --selftest
# ============================================================
set -uo pipefail
SELF_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$SELF_DIR/../.." && pwd)
KNOWN="$ROOT/packaging/ci_known_red.tsv"
ALIAS="$ROOT/packaging/ci_step_alias.tsv"
FIXTURE=""; QUIET=0

while [ $# -gt 0 ]; do
    case "$1" in
        --fixture) shift; FIXTURE="${1:-}";;
        --known)   shift; KNOWN="${1:-$KNOWN}";;
        # M431 · 缺陷 888：CI 的 step 名与门名不是同一命名体系（ci.yml 里用中文描述），
        #   拿 step 名直接查门台账 ⇒ 「门超时口径守卫」永远「未登记」。
        #   本参数允许显式传空串（自测用）：空串 = 不读别名表，而不是「用默认表」。
        --alias)   shift; ALIAS="${1:-}";;
        --quiet)   QUIET=1;;
        --selftest) SELFTEST=1;;
        *) echo "未知参数：$1" >&2; exit 2;;
    esac
    shift
done

TMP=$(mktemp -d /tmp/ciparity.XXXXXX); trap 'rm -rf "$TMP"' EXIT

# ── 取数 ───────────────────────────────────────────────────
fetch(){
    if [ -n "$FIXTURE" ]; then
        [ -f "$FIXTURE/runs.json" ] || return 4
        cp "$FIXTURE/runs.json" "$TMP/runs.json"
        [ -f "$FIXTURE/jobs.json" ] && cp "$FIXTURE/jobs.json" "$TMP/jobs.json"
        return 0
    fi
    ( . "$ROOT/packaging/gh_get.sh"
      gh_get "/repos/NanzhanGroup/PuXian/actions/runs?per_page=40&branch=main" "$TMP/runs.json" ) || return $?
    return 0
}
fetch_ci_jobs(){   # $1 = run id
    [ -n "$FIXTURE" ] && return 0
    ( . "$ROOT/packaging/gh_get.sh"
      gh_get "/repos/NanzhanGroup/PuXian/actions/runs/$1/jobs?per_page=100" "$TMP/jobs.json" ) || return $?
    return 0
}

# ── 判定核心（自测与正式跑同一份）───────────────────────────
judge(){
    python3 - "$TMP/runs.json" "${TMP}/jobs.json" "$KNOWN" "${ALIAS:-}" <<'PY'
import json, os, re, sys, datetime
runs_p, jobs_p, known_p, alias_p = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]

def load(p):
    try:
        with open(p, encoding='utf-8') as f: return json.load(f)
    except Exception: return None

runs = load(runs_p)
if not runs or 'workflow_runs' not in runs:
    print("CI-PARITY-ENV-FAIL 取不到 runs.json（测量失败 ≠ 没有红）"); sys.exit(2)

ci = [r for r in runs['workflow_runs'] if r.get('name') == 'PuXian CI' and r.get('head_branch') == 'main']
if not ci:
    print("CI-PARITY-ENV-FAIL runs.json 里没有 main 上的 PuXian CI 记录"); sys.exit(2)
run = ci[0]
sha = (run.get('head_sha') or '')[:7]
conc = run.get('conclusion') or run.get('status')

known = {}
if os.path.exists(known_p):
    for ln in open(known_p, encoding='utf-8'):
        ln = ln.strip()
        if not ln or ln.startswith('#'): continue
        parts = ln.split('\t')
        if len(parts) >= 3:
            known[parts[0].strip()] = (parts[1].strip(), parts[2].strip())
today = datetime.date.today().isoformat()

# ── M431 · 缺陷 888：step 名 → 门名 的两步归一 ──────────────────
# 第 1 步：剥括号说明。ci.yml 里的 step 名长这样（括号分全角/半角两种都出现过）：
#   m400_baseline_feedback（门耗时基线的输入回路 · M400 · 缺陷 763）
#   门超时口径守卫（声明 ⇄ 实测基线 · M311 · 缺陷 532）
# 第 2 步：查别名表把「中文描述」翻译成 registry 门名（ci_step_alias.tsv）：
#   门超时口径守卫 → gate_timeouts
#   为什么不用模糊匹配：模糊匹配会让“名字像就放过”变成新的静默通道。
#   别名表是**显式穷举**，新增一行要人写 ⇒ 不会自己长出口子。
alias = {}
if alias_p and os.path.exists(alias_p):
    for ln in open(alias_p, encoding='utf-8'):
        ln = ln.strip()
        if not ln or ln.startswith('#'): continue
        p = ln.split('\t')
        if len(p) >= 2 and p[0].strip() and p[1].strip():
            alias[p[0].strip()] = p[1].strip()

def step_base(name):
    """剥掉括号及其后的说明（全角（ 与半角 ( 都剥）。"""
    for ch in ('（', '('):
        i = name.find(ch)
        if i > 0:
            name = name[:i]
    return name.strip()


if conc == 'success':
    print(f"CI-PARITY-OK sha={sha} ci=success（远端与本地无分歧）"); print(f"nred=0 nreg={len(known)}"); sys.exit(0)

jobs = load(jobs_p) or {}
# 粒度：CI 把全部门塞进一个 job（「工具链自举质量门」）里当 step 跑 ⇒ 红门名在 **step** 上。
# 只用 job 名会把十几门红压成一行「XX 质量门」，等于把信息扔掉（2026-11-18 实测）。
failed = []
for j in jobs.get('jobs', []):
    if j.get('conclusion') in ('success', 'skipped', None):
        continue
    steps = [s.get('name') for s in j.get('steps', []) if s.get('conclusion') == 'failure']
    if steps:
        failed.extend(steps)
    else:
        failed.append(j.get('name'))
gap, reg_ok, expired = [], [], []
for name in failed:
    # 原名 → 剥括号后的主名；主名先查别名表（中文描述 → 门名），再回落前导 slug
    base = step_base(name)
    m = re.match(r'[A-Za-z0-9_.\-]+', base)
    gid = m.group(0) if m else base
    # 候选顺序：别名映射的门名 → 原名 → 主名 → 首段 slug。命中即用，全部落空才是 GAP。
    cands = []
    if base in alias: cands.append(alias[base])
    for c in (name, base, gid):
        if c not in cands: cands.append(c)
    key = next((c for c in cands if c in known), None)
    if key is None:
        if base in alias:
            gap.append(f"未登记：{alias[base]}（已由别名表 {base} → {alias[base]} 翻译，但台账里没有它）")
        else:
            gap.append(f"未登记：{gid}（step：{name}）")
        continue
    why, due = known[key]
    if due and due < today:
        expired.append(f"登记过期 {gid}（到期 {due} · {why}）")
    else:
        reg_ok.append(f"已登记 {gid}（到期 {due} · {why}）")

print(f"CI-PARITY sha={sha} ci={conc} 红门={len(failed)}")
for x in reg_ok: print("  登记在册 " + x)
for x in gap:    print("  GAP     " + x)
for x in expired:print("  GAP     " + x)
if gap or expired:
    print(f"CI-PARITY-GAP nred={len(failed)} gap={len(gap)+len(expired)} nreg={len(reg_ok)}")
    sys.exit(1)
print(f"CI-PARITY-SYNC nred={len(failed)} nreg={len(reg_ok)}（红都在账上；到期前必须修或重新定性）")
sys.exit(0)
PY
}

# ── 自测：四个方向各自要有牙 ────────────────────────────────
if [ "${1:-}" = "--selftest" ] || [ "${SELFTEST:-0}" = "1" ]; then
    T=$(mktemp -d /tmp/ciparity_st.XXXXXX); trap 'rm -rf "$T"' EXIT
    mkdir -p "$T/red" "$T/green" "$T/none"
    pass=0; fail=0
    ck(){ if [ "$2" = "$3" ]; then pass=$((pass+1)); echo "  PASS  $1"; else fail=$((fail+1)); echo "  FAIL  $1（期望 $3 实得 $2）"; fi; }
    run_case(){ # $1 fixture dir  $2 known file  $3 alias file（空串 = 明确不读别名表） → echo rc
        local fx="$1" kn="$2" al="${3-}"
        FIXTURE="$fx" KNOWN="$kn" bash "$SELF_DIR/ci_parity_check.sh" --fixture "$fx" --known "$kn" --alias "$al" --quiet >/dev/null 2>&1
        echo $?
    }
    cat > "$T/red/runs.json" <<'J'
{"workflow_runs":[{"name":"PuXian CI","head_branch":"main","head_sha":"deadbeefcafe","conclusion":"failure","status":"completed"}]}
J
    cat > "$T/red/jobs.json" <<'J'
{"jobs":[{"name":"工具链自举质量门","conclusion":"failure"},{"name":"m256_eintr","conclusion":"failure"}]}
J
    cat > "$T/green/runs.json" <<'J'
{"workflow_runs":[{"name":"PuXian CI","head_branch":"main","head_sha":"deadbeefcafe","conclusion":"success","status":"completed"}]}
J
    : > "$T/empty.tsv"
    printf '工具链自举质量门\t夹具：编排步\t2026-12-31\nm256_eintr\t夹具：负载敏感\t2026-12-31\n' > "$T/ok.tsv"
    printf '工具链自举质量门\t夹具：编排步\t2026-12-31\nm256_eintr\t夹具：负载敏感\t2026-01-01\n' > "$T/expired.tsv"
    printf '工具链自举质量门\t夹具：编排步\t2026-12-31\n' > "$T/partial.tsv"

    ck "正控：未登记的红 ⇒ GAP(1)"        "$(run_case "$T/red" "$T/empty.tsv")"   1
    ck "正控：只登记一半 ⇒ GAP(1)"        "$(run_case "$T/red" "$T/partial.tsv")" 1
    ck "负控：全部登记且未过期 ⇒ SYNC(0)" "$(run_case "$T/red" "$T/ok.tsv")"      0
    ck "负控：登记过期 ⇒ GAP(1)"          "$(run_case "$T/red" "$T/expired.tsv")" 1
    ck "负控：CI 绿 ⇒ OK(0)"              "$(run_case "$T/green" "$T/empty.tsv")" 0
    ck "环境：无 fixture ⇒ ENV-FAIL(2)"   "$(run_case "$T/none" "$T/empty.tsv")"  2

    # ── M431 · 缺陷 888：step 名归一化 / 别名表 ──
    # 每个用例只考一件事（多件事混在一份 fixture 里，红绿都说不清是谁造成的）。
    mkdir -p "$T/red_slug" "$T/red_cn"
    for d in red_slug red_cn; do
        cat > "$T/$d/runs.json" <<'J'
{"workflow_runs":[{"name":"PuXian CI","head_branch":"main","head_sha":"deadbeefcafe","conclusion":"failure","status":"completed"}]}
J
    done
    cat > "$T/red_slug/jobs.json" <<'J'
{"jobs":[{"name":"工具链自举质量门（fmt + lint）","conclusion":"failure","steps":[{"name":"m400_baseline_feedback（门耗时基线的输入回路 · M400 · 缺陷 763）","conclusion":"failure"}]}]}
J
    cat > "$T/red_cn/jobs.json" <<'J'
{"jobs":[{"name":"工具链自举质量门（fmt + lint）","conclusion":"failure","steps":[{"name":"门超时口径守卫（声明 ⇄ 实测基线 · M311 · 缺陷 532）","conclusion":"failure"}]}]}
J
    printf 'm400_baseline_feedback\t夹具：剥括号后对上\t2026-12-31\n' > "$T/k_slug.tsv"
    printf 'gate_timeouts\t夹具：别名翻译后对上\t2026-12-31\n' > "$T/k_gate.tsv"
    printf '门超时口径守卫\tgate_timeouts\n' > "$T/alias.tsv"

    ck "F1 剥括号：slug（长说明）在册 ⇒ SYNC(0)"          "$(run_case "$T/red_slug" "$T/k_slug.tsv" "")" 0
    ck "F1b 前提自证：台账换成空 ⇒ 同一 fixture 必红"       "$(run_case "$T/red_slug" "$T/empty.tsv" "")" 1
    ck "F2 别名：中文 step → 门名在册 ⇒ SYNC(0)"            "$(run_case "$T/red_cn" "$T/k_gate.tsv" "$T/alias.tsv")" 0
    ck "F3 无别名表 ⇒ 同一中文 step 必报 GAP(1)"            "$(run_case "$T/red_cn" "$T/k_gate.tsv" "")" 1
    ck "F4 别名指向的门未入账 ⇒ 仍 GAP(1)（别名不洗白）"    "$(run_case "$T/red_cn" "$T/empty.tsv" "$T/alias.tsv")" 1
    echo
    if [ "$fail" = 0 ]; then echo "CI-PARITY-SELFTEST-OK pass=$pass"; exit 0; fi
    echo "CI-PARITY-SELFTEST-FAIL pass=$pass fail=$fail"; exit 1
fi

# ── 正式跑 ─────────────────────────────────────────────────
fetch; rc=$?
if [ "$rc" != 0 ]; then
    echo "CI-PARITY-ENV-FAIL 取数失败 rc=$rc（令牌失效 / 网络 / 无 fixture）——测量失败不等于没有红"
    exit 2
fi
run_id=$(python3 -c 'import json,sys;d=json.load(open(sys.argv[1]));r=[x for x in d["workflow_runs"] if x.get("name")=="PuXian CI" and x.get("head_branch")=="main"];print(r[0]["id"] if r else "")' "$TMP/runs.json" 2>/dev/null || echo "")
if [ -n "$run_id" ] && [ -z "$FIXTURE" ]; then
    fetch_ci_jobs "$run_id" || { echo "CI-PARITY-ENV-FAIL 取不到 jobs.json（run=$run_id）"; exit 2; }
fi
judge
