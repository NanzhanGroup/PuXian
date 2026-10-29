#!/usr/bin/env bash
# ============================================================
# release_reconcile.sh —— 发布通道对账（东月 M278）
#
# 为什么有它（2026-10-06 实测事故）：
#   `release.yml` 用**单通道** concurrency（group=Release-publish，cancel-in-progress=false）。
#   GitHub 的语义是「同组同时只允许一个 in-progress + **一个** pending」——
#   后来的 pending 会**取消**先前的 pending。于是一次性推多个 tag（如历史回填）时，
#   先到的 Release run 被**静默取消**：实测 v0.2.275 的 run 判 cancelled（rpm 三个 job 从未跑）、
#   v0.2.263/264/265/271 同款 → **这些 tag 至今没有任何 Release**。
#   关键：**发布被取消，而没有任何东西会告诉你**（CI 不报、Tag Guard 不报、mirror 不报）。
#
# 它做什么：把「tag ⇄ Release ⇄ 资产」对账，找出**该有发布却没有**的 tag，
#   并（`--rerun`）把 **cancelled** 的 run 重新跑起来。
#   ⚠️ 只重跑 cancelled —— failed 可能是真失败，机械重跑会掩盖问题（要人看）。
#
# 用法：
#   release_reconcile.sh [--repo OWNER/NAME] [--limit N] [--rerun] [--json]
#                        [--ignore FILE]         # 豁免表（默认 packaging/release_reconcile.ignore）
#                        [--api-fixture DIR]     # 离线：读 DIR/{releases,runs}.json 而非联网
#   环境：GH_TOKEN（**必须来自环境变量**，不落盘、不进仓库 —— [SECURITY/P0]）
#
# 判据（VERDICT 列）：
#   OK                     有 Release 且含主包 tarball + sha256sums.txt
#   ASSETS-INCOMPLETE      有 Release 但缺关键资产（或 run 成功却没建出 Release）
#   NO-RELEASE-CANCELLED   run 被取消（可 --rerun 救）
#   NO-RELEASE-FAILED      run 失败（**要人看**，不自动重跑）
#   NO-RELEASE-NORUN       该 tag 没有任何 Release run（未触发 / 被判据跳过）
#   NO-RELEASE-INPROGRESS  run 还在跑（**不算异常**）
#   LATEST-STALE           releases/latest ≠ 最高版本 tag ⇒ install.sh（取 releases/latest）
#                          会给用户装**旧树**（缺陷 587 修复新增）
#   KNOWN-NO-RELEASE       在豁免表里、且理由非空 ⇒ 不计入异常（历史回填 tag 的出口）
#   STALE-IGNORE           ⚠ 豁免表里写着，但它**就是最新的版本 tag** ⇒ 豁免过期（最新必须有发布）
# 退出码：0 = 无异常；1 = 有异常；2 = 用法/环境错
# ============================================================
set -uo pipefail

REPO=${GH_REPO:-NanzhanGroup/PuXian}
LIMIT=30
RERUN=0
JSON=0
FIXTURE=""
API=${GH_API:-https://api.github.com}
HERE=$(cd "$(dirname "$0")" && pwd)
IGNORE=${GH_IGNORE:-$HERE/release_reconcile.ignore}

usage(){ sed -n '2,44p' "$0" | sed 's/^# \{0,1\}//'; }

while [ $# -gt 0 ]; do
  case "$1" in
    --repo)        REPO="$2"; shift 2;;
    --limit)       LIMIT="$2"; shift 2;;
    --rerun)       RERUN=1; shift;;
    --json)        JSON=1; shift;;
    --ignore)      IGNORE="$2"; shift 2;;
    --api-fixture) FIXTURE="$2"; shift 2;;
    -h|--help)     usage; exit 0;;
    *) echo "未知选项：$1" >&2; usage >&2; exit 2;;
  esac
done

fetch(){
    local path="$1" file="$2"
    if [ -n "$FIXTURE" ]; then
        cat "$FIXTURE/$file" 2>/dev/null || { echo "❌ fixture 缺 $file" >&2; return 1; }
        return 0
    fi
    [ -n "${GH_TOKEN:-}" ] || { echo "❌ 未设置 GH_TOKEN（令牌只走环境变量）" >&2; return 2; }
    curl -sS -H "Authorization: Bearer $GH_TOKEN" -H "Accept: application/vnd.github+json" \
         "$API/repos/$REPO$path" || return 1
}

W=$(mktemp -d); trap 'rm -rf "$W"' EXIT

# ---- 拉数据（各一次请求，不逐 tag 打 API —— 免配额纪律）----
fetch "/releases?per_page=100"  releases.json > "$W/rel.json" || exit 2
fetch "/actions/workflows/release.yml/runs?per_page=100" runs.json > "$W/run.json" || exit 2

# ---- 第二数据源：git tag 列表（缺陷 586 修复）----
# 为什么必须加：原实现 tags = 有 Release 的 ∪ 有 run 的 ⇒「**既无 Release 又无 run**」的 tag
#   根本不在集合里，于是这种 tag 永远显示不出来。2026-10-26 的事故正是这一类：
#   v0.2.325/326/327/330 推上去后 tag 事件没被 Actions 派发（0 个 run、0 个 Release），
#   本工具当时照样报 RELEASE-RECONCILE-OK —— 判据看不见它要判的东西。
# 作用域（不加作用域就会被历史噪声淹没，实测数据支撑）：
#   远端 240 个 tag 中 117 个无 Release，其中 v0.2.* 占 96 个，绝大多数是
#   v0.2.0-mNN / s3 这类**每里程碑开发 tag**（本就不该有可下载版本）。
#   ⇒ 只有「**纯数字版本号** ^v<主>.<次>.<修>$」算正式发布 tag，才要求正文发布；
#     带后缀的开发 tag 不进要求集。实测：纯数字 tag 无 Release 的只有 5 个
#     （v0.2.261/263/264/265/271），且全部已在豁免表 ⇒ 本修复上线后对账仍 0 异常，不造假红。
TAG_FILES=""
for pg in 1 2; do
    f="$W/tags$pg.json"
    if [ -n "$FIXTURE" ]; then
        src="$FIXTURE/tags.json"; [ "$pg" = 2 ] && src="$FIXTURE/tags2.json"
        [ -f "$src" ] && { cp "$src" "$f"; TAG_FILES="$TAG_FILES $f"; }
    elif curl -sS -H "Authorization: Bearer $GH_TOKEN" -H "Accept: application/vnd.github+json" \
             "$API/repos/$REPO/tags?per_page=100&page=$pg" > "$f" 2>/dev/null && grep -q '\[' <<< "$(head -c1 "$f" 2>/dev/null)"; then
        TAG_FILES="$TAG_FILES $f"
    fi
done
[ -n "$TAG_FILES" ] || echo "⚠️ [586] 拿不到 git tag 列表 ⇒ 本次无法发现「既无 Release 又无 run」的 tag（能力降级）" >&2

# ---- releases/latest（缺陷 587 修复）----
# install.sh 取 api/repos/.../releases/latest ⇒ latest 指错版本 = 一键安装装旧树。
#   2026-10-26 实测事故：先补 330 再补 325/326/327，GitHub 的 latest 按**创建时间**排、
#   不看版本号 ⇒ latest 一度变成 v0.2.327，而当时对账报 OK。
LATEST_FILE=""
if [ -n "$FIXTURE" ]; then
    [ -f "$FIXTURE/latest.json" ] && LATEST_FILE="$FIXTURE/latest.json"
else
    if curl -sS -H "Authorization: Bearer $GH_TOKEN" -H "Accept: application/vnd.github+json" \
         "$API/repos/$REPO/releases/latest" > "$W/latest.json" 2>/dev/null && grep -q '"tag_name"' "$W/latest.json"; then
        LATEST_FILE="$W/latest.json"
    fi
fi
[ -n "$LATEST_FILE" ] || echo "⚠️ [587] 拿不到 releases/latest ⇒ latest 与最高版本 tag 的一致性校验跳过（能力降级）" >&2

# 豁免表：<tag>\t<理由>（# 注释、空行忽略）。理由为空视为**无效条目** ⇒ 不当豁免。
: > "$W/ign"; IGN_BAD=0
if [ -f "$IGNORE" ]; then
    while IFS= read -r line; do
        case "$line" in ''|\#*) continue;; esac
        t=${line%%$'\t'*}; r=${line#*$'\t'}
        if [ "$r" = "$line" ] || [ -z "${r// }" ]; then IGN_BAD=$((IGN_BAD+1)); continue; fi
        printf '%s\t%s\n' "$t" "$r" >> "$W/ign"
    done < "$IGNORE"
fi

TAG_FILES="$TAG_FILES" LATEST_FILE="$LATEST_FILE" LATEST_OUT="$W/latest.tsv" \
python3 - "$W/rel.json" "$W/run.json" "$LIMIT" "$W/ign" <<'PY' > "$W/table.tsv"
import json, sys, re, os
rel = json.load(open(sys.argv[1]))
runs = json.load(open(sys.argv[2]))
limit = int(sys.argv[3])
ign = {}
for ln in open(sys.argv[4]):
    ln = ln.rstrip('\n')
    if '\t' in ln:
        t, r = ln.split('\t', 1); ign[t] = r

def is_ver(t): return bool(re.match(r'^v\d', t or ''))
def is_release_tag(t): return bool(re.match(r'^v\d+\.\d+\.\d+$', t or ''))
def vkey(t):
    nums = [int(x) for x in re.findall(r'\d+', t.split('-')[0])]
    return (tuple((nums + [0]*4)[:4]), t)

relmap = {}
for r in (rel if isinstance(rel, list) else []):
    if is_ver(r.get('tag_name')):
        relmap[r['tag_name']] = r

# 同一 tag 多个 run 时保留「最好的」结论（success 最优）
rank = {'success':0, 'in_progress':1, 'queued':1, 'pending':1, 'cancelled':2, 'failure':3}
runmap = {}
for r in (runs.get('workflow_runs', []) if isinstance(runs, dict) else []):
    t = r.get('head_branch')
    if not is_ver(t): continue
    cur = runmap.get(t)
    k = lambda x: rank.get(x.get('conclusion'), 4)
    if cur is None or k(r) < k(cur): runmap[t] = r

# git tag 第二源：只收「纯数字版本号」的正式发布 tag（作用域理由见脚本上半段注释）
extra = set()
for f in os.environ.get('TAG_FILES', '').split():
    try:
        d = json.load(open(f))
    except Exception:
        continue
    if isinstance(d, list):
        for it in d:
            n = (it or {}).get('name')
            if is_release_tag(n): extra.add(n)

tags = sorted(set(relmap) | set(runmap) | extra, key=vkey, reverse=True)
newest = tags[0] if tags else None

# releases/latest 与「最高版本 tag」的一致性（缺陷 587）：结果交给 shell 判红
_out = os.environ.get('LATEST_OUT', '')
if _out:
    lt = ''
    lp = os.environ.get('LATEST_FILE', '')
    if lp:
        try: lt = (json.load(open(lp)) or {}).get('tag_name') or ''
        except Exception: lt = ''
    rel_only = [t for t in tags if is_release_tag(t)]
    exp = rel_only[0] if rel_only else ''
    with open(_out, 'w') as fh:
        fh.write('%s\t%s\n' % (lt, exp))
for t in tags[:limit]:
    r, run = relmap.get(t), runmap.get(t)
    assets = [a['name'] for a in (r or {}).get('assets', [])]
    has_main = any(a.endswith('.tar.gz') and 'bootstrap' not in a for a in assets)
    has_sums = 'sha256sums.txt' in assets
    if r is not None:
        verdict = 'OK' if (has_main and has_sums) else 'ASSETS-INCOMPLETE'
    elif run is None:
        verdict = 'NO-RELEASE-NORUN'
    else:
        c = run.get('conclusion')
        if run.get('status') != 'completed':
            verdict = 'NO-RELEASE-INPROGRESS'
        elif c == 'cancelled':  verdict = 'NO-RELEASE-CANCELLED'
        elif c == 'success':    verdict = 'ASSETS-INCOMPLETE'   # run 成功却无 Release ⇒ 创建步出问题
        else:                   verdict = 'NO-RELEASE-FAILED'
    # 豁免：只对「确实没有 Release」的判定生效；最新 tag 的豁免视为**过期**
    if verdict.startswith('NO-RELEASE') and t in ign:
        verdict = 'STALE-IGNORE' if t == newest else 'KNOWN-NO-RELEASE'
    print('\t'.join([t,
                     (run or {}).get('head_sha','')[:8],
                     str((run or {}).get('id','-')),
                     # 未完成时 conclusion 为 None ⇒ 显示 status（RUN_CONCL 列不该出现 "None"）
                     str((run or {}).get('conclusion') or (run or {}).get('status') or '-'),
                     'Y' if r is not None else 'N',
                     str(len(assets)),
                     verdict,
                     ign.get(t, '')]))
PY
[ -s "$W/table.tsv" ] || { echo "❌ 对账表为空（API 返回异常？）" >&2; exit 2; }

# ---- latest 一致性（缺陷 587）：latest ≠ 最高版本 tag ⇒ 异常 ----
LATEST_TAG=""; LATEST_EXP=""; LATEST_BAD=0
if [ -s "$W/latest.tsv" ]; then
    IFS=$'\t' read -r LATEST_TAG LATEST_EXP < "$W/latest.tsv"
fi
if [ -n "${LATEST_EXP:-}" ] && [ "${LATEST_TAG:-}" != "$LATEST_EXP" ]; then
    LATEST_BAD=1
fi

# ---- 重跑 cancelled（只这一类；failed 要人看）----
RERUN_DONE=""; RERUN_FAIL=""
if [ "$RERUN" = 1 ]; then
    while IFS=$'\t' read -r tag sha rid concl hasrel nass verdict why; do
        [ "$verdict" = "NO-RELEASE-CANCELLED" ] || continue
        [ "$rid" != "-" ] || continue
        if [ -n "$FIXTURE" ]; then RERUN_DONE="$RERUN_DONE $tag(fixture)"; continue; fi
        code=$(curl -sS -o /dev/null -w '%{http_code}' -X POST \
               -H "Authorization: Bearer $GH_TOKEN" -H "Accept: application/vnd.github+json" \
               "$API/repos/$REPO/actions/runs/$rid/rerun")
        case "$code" in
          201|202|204) RERUN_DONE="$RERUN_DONE $tag";;
          *)           RERUN_FAIL="$RERUN_FAIL $tag(HTTP $code)";;
        esac
    done < "$W/table.tsv"
fi

BAD=$(awk -F'\t' '$7!="OK" && $7!="NO-RELEASE-INPROGRESS" && $7!="KNOWN-NO-RELEASE"' "$W/table.tsv" | wc -l)
if [ "$IGN_BAD" != 0 ]; then BAD=$((BAD+1)); fi
if [ "$LATEST_BAD" != 0 ]; then BAD=$((BAD+1)); fi

if [ "$JSON" = 1 ]; then
    awk -F'\t' -v bad="$BAD" -v repo="$REPO" -v rd="$RERUN_DONE" -v rf="$RERUN_FAIL" -v ib="$IGN_BAD" \
        -v lt="${LATEST_TAG:-}" -v le="${LATEST_EXP:-}" -v ls="$LATEST_BAD" '
      BEGIN{ printf "{\"repo\":\"%s\",\"bad\":%d,\"ignore_bad_entries\":%d,\"latest\":\"%s\",\"latest_expected\":\"%s\",\"latest_stale\":%d,\"rerun_ok\":\"%s\",\"rerun_fail\":\"%s\",\"items\":[", repo, bad, ib, lt, le, ls, rd, rf }
      { printf "%s{\"tag\":\"%s\",\"sha\":\"%s\",\"run_id\":\"%s\",\"conclusion\":\"%s\",\"release\":\"%s\",\"assets\":%s,\"verdict\":\"%s\"}",
          (NR>1?",":""), $1,$2,$3,$4,$5,$6,$7 }
      END{ print "]}" }' "$W/table.tsv"
else
    printf '%-18s %-9s %-11s %-11s %-3s %-4s %s\n' TAG SHA RUN_ID RUN_CONCL REL 资产 判定
    awk -F'\t' '{ printf "%-18s %-9s %-11s %-11s %-3s %-4s %s\n", $1,$2,$3,$4,$5,$6,$7 }' "$W/table.tsv"
    echo
    if [ "$LATEST_BAD" = 1 ]; then
        echo "⚠️  LATEST-STALE: releases/latest=${LATEST_TAG:-（取不到）} ≠ 最高版本 tag $LATEST_EXP —— install.sh 取 latest ⇒ 会给用户装旧树"
    elif [ -n "${LATEST_EXP:-}" ]; then
        echo "latest 校验：releases/latest = $LATEST_TAG（= 最高版本 tag $LATEST_EXP）✅"
    fi
    echo "对账：$(wc -l < "$W/table.tsv") 个版本 tag · 异常 $BAD 个 · 豁免 $(wc -l < "$W/ign") 条"
    [ "$IGN_BAD" != 0 ] && echo "⚠️ 豁免表有 $IGN_BAD 条**理由为空**的无效行（不当豁免）"
    [ -n "$RERUN_DONE" ] && echo "已重跑 cancelled：$RERUN_DONE"
    [ -n "$RERUN_FAIL" ] && echo "⚠️ 重跑失败：$RERUN_FAIL"
    [ "$BAD" = 0 ] && echo "RELEASE-RECONCILE-OK" || echo "RELEASE-RECONCILE-WARN"
fi
[ "$BAD" = 0 ] && exit 0 || exit 1
