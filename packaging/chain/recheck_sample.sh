#!/usr/bin/env bash
# ============================================================
# packaging/chain/recheck_sample.sh —— 语义轮「抽样串行复核」（M317 · 用户令方案②）
# ------------------------------------------------------------
# 来历（用户令 2026-10-21）：
#   原口径「语义面命中 ⇒ SERIAL（串行权威档）」实测 5.2h（M316 18774s），
#   而机器实测 CPU 利用率仅 12.6%（1.01 核 / 8 核）——慢的不是门，是「一次只跑一个门」。
#   改为：**分片全覆盖（~98min）+ 抽样串行复核（~40min）**。
#
# 判据不是「降级」，是「把权威性集中在最需要它的门上」：
#   抽样集（**源码派生，不手抄**）=
#     ① Top-K 耗时门（从**本轮分片实测**的 times.tsv 排序取，默认 K=12）
#        —— 资源竞争最敏感的一批；并行若影响结果，最先在它们身上露出来
#     ② quiet_needed.txt 全部（它们本就被摘出分片、在机器空闲时串行跑 ⇒ 天然是权威）
#   对拍：抽样门「分片结果」⇄「串行结果」逐门 (状态, rc) 必须一致；不一致 ⇒ RED。
#
# 用法：
#   recheck_sample.sh <分片日志目录> [--top K] [--dry] [--quiet]
#   输出：<日志目录>/recheck.times.tsv · recheck.log · recheck.verdict.txt
#   退出码：0 = GREEN（抽样全绿且与分片逐门一致）· 1 = RED · 3 = 前提不成立（无法判定）
#
# ⚠️ 三条纪律：
#   1) 本脚本**必须在分片全部结束后**跑 —— 抽样门的判据前提是「机器空闲」，
#      与 quiet 档同理（quiet_needed.txt 的注册理由就是这么写的）。
#   2) 抽样门若在分片侧**没有结果**（SKIP/缺失），不许"跳过算绿" —— 响亮报 rc=3。
#   3) 本脚本只**读**产品的门，不改产品代码；失败归因走 gate_verdict.sh。
# ============================================================
set -uo pipefail

# 第一个参数若是选项（--xxx）则视为「无日志目录」（--self-test 走这条）
case "${1:-}" in
    --*) LOG="" ;;
    *)   LOG="${1:-}"; shift || true ;;
esac
TOP="${RECHECK_TOP:-12}"
DRY=0
QUIET=0
SELF_TEST=0
while [ $# -gt 0 ]; do
    case "$1" in
        --top)   TOP="${2:-12}"; shift 2 ;;
        --dry)   DRY=1; shift ;;
        --quiet) QUIET=1; shift ;;
        --self-test) SELF_TEST=1; shift ;;
        *) echo "未知参数：$1" >&2; exit 4 ;;
    esac
done

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
BASE="/data/gatevm"
QUIET_FILE="${QUIET_FILE:-$BASE/quiet_needed.txt}"

# --self-test 不需要日志目录（它自己造合成夹具）
if [ "$SELF_TEST" != "1" ]; then
    [ -n "$LOG" ] && [ -d "$LOG" ] || { echo "用法：recheck_sample.sh <分片日志目录> [--top K] [--self-test]" >&2; exit 4; }
fi

say() { [ "$QUIET" = 1 ] || echo "$@"; }

# ══════════════════════════════════════════════════════════════
# 自证（--self-test）：判据本身有没有牙
#   S1 全一致              ⇒ GREEN
#   S2 状态不一致          ⇒ RED（且指名哪一门）
#   S3 串行侧红门          ⇒ RED
#   S4 分片侧缺结果        ⇒ RED（**不许跳过算绿** —— 本脚本最容易被"优化"掉的一条）
#   做法：造 4 组合成夹具，走**同一条对拍管线**（只把"跑门"换成 RECHECK_NO_RUN=1）
# ══════════════════════════════════════════════════════════════
if [ "$SELF_TEST" = "1" ]; then
    echo "══ recheck_sample 自证 ══"
    PASS=0; FAIL=0
    TMPD="$(mktemp -d)"; trap 'rm -rf "$TMPD"' EXIT
    mkfx() {  # $1=dir  $2=分片times内容  $3=串行times内容
        mkdir -p "$1"
        for i in 1 2 3 4; do
            echo 0 > "$1/shard$i.rc"
            printf '%b\n' "$2" > "$1/shard$i.times.tsv"
        done
        printf '%b\n' "$3" > "$1/recheck.times.tsv"
    }
    # quiet 表固定为 m295（夹具里作为"抽样门"之一）
    QF="$TMPD/quiet.txt"; echo "m295_quic_slot" > "$QF"

    run_case() {  # $1=name $2=分片 $3=串行 $4=期望(0/1) $5=期望关键词
        local d="$TMPD/$1"
        mkfx "$d" "$2" "$3"
        local out rc
        out="$(QUIET_FILE="$QF" RECHECK_NO_RUN=1 RECHECK_TOP=1 bash "$0" "$d" 2>&1)"; rc=$?
        if [ "$rc" = "$4" ] && printf '%s' "$out" | grep -q "$5"; then
            PASS=$((PASS+1)); echo "  ✅ $1：rc=$rc 且命中「$5」"
        else
            FAIL=$((FAIL+1)); echo "  ❌ $1：rc=$rc（期望 $4）· 未见「$5」"; printf '%s\n' "$out" | tail -12 | sed 's/^/       /'
        fi
    }

    # S1 全一致（upstream_tests 是 Top-1 耗时门；m295 是 quiet 门）
    run_case s1_all_ok \
"upstream_tests\tOK\t900\t0
m295_quic_slot\tOK\t120\t0" \
"upstream_tests\tOK\t950\t0
m295_quic_slot\tOK\t130\t0" 0 "RECHECK-VERDICT-GREEN"

    # S2 状态不一致（同一门：分片 OK / 串行 FAIL）
    run_case s2_state_diff \
"upstream_tests\tOK\t900\t0
m295_quic_slot\tOK\t120\t0" \
"upstream_tests\tFAIL\t950\t1
m295_quic_slot\tOK\t130\t0" 1 "状态/rc 不一致      : 1"

    # S3 串行侧红门（两侧都 FAIL 也算红 —— 抽样集里出现红就是 RED）
    run_case s3_serial_red \
"upstream_tests\tFAIL\t900\t1
m295_quic_slot\tOK\t120\t0" \
"upstream_tests\tFAIL\t950\t1
m295_quic_slot\tOK\t130\t0" 1 "串行侧红门"

    # S4 分片侧缺结果（抽样门在分片里没跑出来 ⇒ 必须 RED，不许算绿）
    run_case s4_missing_par \
"other_gate\tOK\t10\t0" \
"upstream_tests\tOK\t950\t0
m295_quic_slot\tOK\t130\t0" 1 "分片侧缺结果"

    echo
    if [ "$FAIL" = "0" ]; then echo "══ RECHECK-SELFTEST-OK · $PASS 通过 / 0 失败 ══"; exit 0
    else echo "══ RECHECK-SELFTEST-FAIL · $PASS 通过 / $FAIL 失败 ══"; exit 1; fi
fi

# ── 1) 前提自证：分片确实跑完了（不允许在分片还在跑时开工）──
for i in 1 2 3 4; do
    [ -f "$LOG/shard$i.rc" ] || { echo "❌ 前提不成立：$LOG/shard$i.rc 不存在（分片未跑完）" >&2; exit 3; }
done
say "前提自证 ✅ 4 个分片都有 rc 落盘"

# ── 2) 本轮分片实测耗时（合并 4 片；同门取真正跑过的那一行）──
MERGED="$LOG/times.merged.tsv"
python3 - "$LOG" "$MERGED" <<'PY'
import sys, os, glob
log, out = sys.argv[1], sys.argv[2]
best = {}
for p in sorted(glob.glob(os.path.join(log, 'shard*.times.tsv'))):
    for ln in open(p, encoding='utf-8', errors='ignore'):
        f = ln.rstrip('\n').split('\t')
        if len(f) < 2: continue
        name, status = f[0], f[1]
        try: sec = int(float(f[2])) if len(f) > 2 and f[2] not in ('', '-') else 0
        except ValueError: sec = 0
        if status == 'SKIP': continue
        if name not in best or sec > best[name][0]:
            best[name] = (sec, status, f[3] if len(f) > 3 else '-')
with open(out, 'w', encoding='utf-8') as fh:
    for name in sorted(best):
        sec, st, rc = best[name]
        fh.write('%s\t%s\t%d\t%s\n' % (name, st, sec, rc))
print('合并实测门数：%d' % len(best))
PY

# ── 3) 抽样集（源码派生）：Top-K 耗时门 ∪ quiet 档 ──
python3 - "$MERGED" "$QUIET_FILE" "$TOP" "$LOG" <<'PY'
import sys, os
merged, qfile, top, log = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]

rows = []
for ln in open(merged, encoding='utf-8'):
    f = ln.rstrip('\n').split('\t')
    if len(f) >= 3:
        try: rows.append((int(f[2]), f[0]))
        except ValueError: pass
rows.sort(reverse=True)

quiet = []
if os.path.exists(qfile):
    for l in open(qfile, encoding='utf-8'):
        l = l.split('#')[0].strip()
        if l: quiet.append(l)
quiet_set = set(quiet)

# ① Top-K（排除静音档 —— 它们本来就在分片后串行跑过，其结果已在 quiet.times.tsv）
topk = [g for _, g in rows if g not in quiet_set][:top]
# ② quiet 全部（要求确实在分片日志里有记录，否则说明表与运行不一致）
sample = topk + [g for g in quiet if g in {x[1] for x in rows}]
open(os.path.join(log, 'recheck.sample.txt'), 'w', encoding='utf-8').write('\n'.join(sample) + '\n')
open(os.path.join(log, 'recheck.topk.txt'), 'w', encoding='utf-8').write('\n'.join(topk) + '\n')
print('抽样集 %d 门（Top-%d 耗时 %d ∪ quiet %d）' % (len(sample), top, len(topk), len(sample) - len(topk)))
for t, g in rows[:top + 3]:
    print('   %6ds  %s%s' % (t, g, '  ← 抽样' if g in set(sample) else ''))
PY

SAMPLE="$(paste -sd, "$LOG/recheck.sample.txt")"
NS=$(wc -l < "$LOG/recheck.sample.txt")
say "抽样门数：$NS"

if [ "$DRY" = 1 ]; then
    say "（--dry：不启动；抽样清单 $LOG/recheck.sample.txt）"
    exit 0
fi

# ── 4) 串行跑抽样集（在分片工作树，机器已空闲）──
#    worktree 与分片同一份代码（HEAD）；先恢复可能被负控改过的受管目录
WT="$BASE/wt1"
if [ "${RECHECK_NO_RUN:-0}" = "1" ]; then
    :   # 自证/复用档：不碰工作树
else
    [ -d "$WT" ] || { echo "❌ 前提不成立：$WT 不存在（分片工作树已被清）" >&2; exit 3; }
    git -C "$WT" checkout -q -- selfhost runtime tools 2>/dev/null || true
fi

if [ "${RECHECK_NO_RUN:-0}" = "1" ]; then
    # 自证用：跳过真跑，直接用夹具里的 recheck.times.tsv（判据层与真跑完全相同）
    say "（RECHECK_NO_RUN=1：跳过真跑，直读 $LOG/recheck.times.tsv）"
else
    say "── 串行复核开始（$NS 门，机器应已空闲）──"
    T0=$(date +%s)
    ( cd "$WT" && GATE_TSV="$LOG/recheck.times.tsv" GATE_TIMEOUT_DEFAULT=900 \
        bash selfhost/run_gates.sh --only "$SAMPLE" ) > "$LOG/recheck.log" 2>&1
    RC=$?
    T1=$(date +%s)
    say "── 串行复核结束 rc=$RC · 耗时 $((T1-T0))s ──"
fi

# ── 5) 对拍：抽样门 分片 ⇄ 串行（逐门 状态 + rc）──
python3 - "$MERGED" "$LOG/recheck.times.tsv" "$LOG/recheck.sample.txt" "$LOG" <<'PY'
import sys, os
merged, serial, sample_f, log = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]

def load_par(p):
    d = {}
    for ln in open(p, encoding='utf-8', errors='ignore'):
        f = ln.rstrip('\n').split('\t')
        if len(f) < 2: continue
        st = f[1]
        if st == 'SKIP': continue
        d[f[0]] = (st, f[3] if len(f) > 3 else '-')
    return d

par = load_par(merged)
ser = load_par(serial)
sample = [l.strip() for l in open(sample_f, encoding='utf-8') if l.strip()]

missing_ser = [g for g in sample if g not in ser]
missing_par = [g for g in sample if g not in par]
red_ser     = [g for g in sample if g in ser and ser[g][0] != 'OK']
diff        = [(g, par[g], ser[g]) for g in sample if g in par and g in ser and par[g] != ser[g]]

lines = []
lines.append('== 抽样串行复核 · 逐门对拍 ==')
lines.append('抽样 %d 门 · 分片有结果 %d · 串行有结果 %d' % (len(sample), len(sample) - len(missing_par), len(sample) - len(missing_ser)))
lines.append('① 串行侧红门          : %d %s' % (len(red_ser), red_ser[:10]))
lines.append('② 分片侧缺结果(禁算绿): %d %s' % (len(missing_par), missing_par[:10]))
lines.append('③ 串行侧缺结果        : %d %s' % (len(missing_ser), missing_ser[:10]))
lines.append('④ 状态/rc 不一致      : %d' % len(diff))
for g, p, s in diff[:12]:
    lines.append('     %s: 分片=%s 串行=%s' % (g, p, s))
ok = (not red_ser) and (not missing_par) and (not missing_ser) and (not diff)
lines.append('RECHECK-VERDICT-' + ('GREEN' if ok else 'RED'))
txt = '\n'.join(lines)
print(txt)
open(os.path.join(log, 'recheck.verdict.txt'), 'w', encoding='utf-8').write(txt + '\n')
sys.exit(0 if ok else 1)
PY
VRC=$?
echo "复核裁决 rc=$VRC（明细 $LOG/recheck.verdict.txt）"
exit "$VRC"
