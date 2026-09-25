#!/usr/bin/env bash
# ============================================================
# M212（第 91 轮）门：**审计器的触发点集合必须源码派生**（缺陷 288/289/290/291）
# ------------------------------------------------------------
# 为什么有这个门：
#   M209 把判据定成「触发点 = 调用链上会 `gc_register` 的入口」，但审计器里那份
#   **手抄**名单（27 个构造器）从没跟着改 ⇒ 漏掉 **300+** 个分配入口
#   （`px_call` 族 · `px_session_read` · `h3_send_fields` · `bi_*` 整族 …）
#   ⇒ 「候选 0」是**假的**（漏报比假阳危险）。
#
# 层：
#   ① 派生器自证（10 锚点：4 必中 / 5 必不中 + 规模下限 + `px_call` 在场）
#   ② 派生集合的**性质**（PRODUCER ⊆ TRIGGER · `px_s3_exec` 是触发点但**不是**生产者）
#   ③ **A/B**：手抄集合（legacy）候选 0 ⇄ 派生集合候选 3 —— 差异即「手抄漏报」的实证
#   ④ 3 条候选 ⇄ `examples/m206_gcroot/BASELINE.tsv` §① 逐条一致（判据不放水）
#   ⑤ 负控 3 道（各自独立判红 + 源逐字节还原）
#   ⑥ 覆盖边界（如实登记）
# 用法：bash examples/m212_gcroot_derive/verify.sh [--neg-skip]
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="$(mktemp -d /tmp/m212_gate.XXXXXX)"
SNAP="$W/snap"
SRC_LIST="selfhost/gcroot_audit.py selfhost/gcroot_derive.py"

snapshot()   { mkdir -p "$SNAP"; for f in $SRC_LIST; do mkdir -p "$SNAP/$(dirname "$f")"; cp "$f" "$SNAP/$f"; done; }
restore_all(){ for f in $SRC_LIST; do [ -f "$SNAP/$f" ] && cp "$SNAP/$f" "$f"; done; }
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad()  { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
step() { echo; echo "=== $1 ==="; }

patch_one() {   # $1=file $2=old $3=new $4=tag（唯一匹配，否则失败）
    python3 - "$1" "$2" "$3" "$4" <<'PY' || return 1
import sys
p, old, new, tag = sys.argv[1:5]
s = open(p, encoding='utf-8').read()
n = s.count(old)
if n != 1:
    print("  !! [%s] 锚点匹配 %d 次（要求 1）" % (tag, n)); sys.exit(1)
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PY
}

# ⚠️ M213（第 92 轮）：锚点数 **10 → 19**（新增：显式 GC 入口族 ⑪/⑭、间接调用分层
#   双向 ⑫/⑮、F1 正/反 ⑯/⑰、真实树 F1 ⑱/⑲）。判据变了⇒本门必须跟着改。
step "① 派生器自证（gcroot_derive.py · 19 锚点）"
if python3 selfhost/gcroot_derive.py --self-test > "$W/derive.log" 2>&1; then
    grep -q 'self-test: 19 通过 / 0 失败' "$W/derive.log" \
        && ok "派生器自证 19/19（含间接调用**分层**双向、显式 GC 族、F1 正/反）" \
        || { bad "自证结论行不符"; tail -14 "$W/derive.log" | sed 's/^/      /'; }
else
    bad "派生器自证失败"; tail -14 "$W/derive.log" | sed 's/^/      /'
fi

step "② 派生集合的性质（源码派生 · 规模 + 集合关系 + 关键成员）"
python3 selfhost/gcroot_derive.py --json > "$W/derive.json" 2> "$W/derive.err" || bad "派生器执行失败"
python3 - "$W/derive.json" <<'PY' > "$W/props.log" 2>&1
import json, sys
d = json.load(open(sys.argv[1], encoding='utf-8'))
T, P, st = set(d['triggers']), set(d['producers']), d['stats']
errs = []
if len(T) < 300: errs.append("TRIGGER 规模下限失败：%d < 300" % len(T))
if len(P) < 250: errs.append("PRODUCER 规模下限失败：%d < 250" % len(P))
if not (P <= T): errs.append("PRODUCER ⊄ TRIGGER")
for nm in ('px_call', 'px_session_read', 'h3_send_fields', 'px_str', 'px_dict'):
    if nm not in T: errs.append("触发点缺关键成员：%s（间接调用/传递闭包失效）" % nm)
# 「触发点 ≠ 生产者」的实证样本：返回 int 的 px_s3_exec
if 'px_s3_exec' not in T: errs.append("px_s3_exec 不在 TRIGGER（它调 px_net_err ⇒ 会分配）")
if 'px_s3_exec' in P: errs.append("px_s3_exec 被当成 PRODUCER（返回 int，会造 4 条假阳）")
print("TRIGGER=%d PRODUCER=%d 直接=%d 间接=%d 函数=%d"
      % (len(T), len(P), st['direct'], st['indirect'], st['funcs']))
print("PASS" if not errs else "FAIL")
for e in errs:
    print("   " + e)
PY
if grep -q '^PASS$' "$W/props.log"; then
    ok "$(head -1 "$W/props.log")（PRODUCER ⊆ TRIGGER · 关键成员在场 · 触发点≠生产者）"
else
    bad "派生集合性质不符"; sed -n '2,8p' "$W/props.log" | sed 's/^/      /'
fi

step "③ A/B：手抄集合（legacy）候选 0 ⇄ 派生集合候选 4"
python3 selfhost/gcroot_audit.py --trigger-set legacy --json > "$W/leg.json" 2> "$W/leg.err" || bad "legacy 扫描失败"
python3 selfhost/gcroot_audit.py --json > "$W/der.json" 2> "$W/der.err" || bad "派生扫描失败"
LEGN=$(python3 -c "import json;print(len(json.load(open('$W/leg.json'))['findings']))" 2>/dev/null)
DERN=$(python3 -c "import json;print(len(json.load(open('$W/der.json'))['findings']))" 2>/dev/null)
[ "${LEGN:-x}" = 0 ] && ok "手抄集合候选 0（= M209 报出的「全仓候选 0」—— 现在知道那是**漏报**）" \
                     || bad "legacy 候选 ${LEGN:-?}（期望 0，手抄集合行为应保持原样）"
# ⚠️ M213：3 → 4（缺陷 293 消 1 / 缺陷 297 F1 消 3 / 缺陷 298 新增 1 ⇒ 4）。
[ "${DERN:-x}" = 4 ] && ok "派生集合候选 4（手抄漏掉的 300+ 入口现在会触发判定）" \
                     || bad "derived 候选 ${DERN:-?}（期望 4）"
grep -q '源码派生' "$W/der.err" && ok "stderr 打印「源码派生」诊断行（缺它=悄悄退回手抄集合）" \
                                 || bad "未打印派生诊断行"

step "④ 4 条候选 ⇄ m206 BASELINE §① 逐条一致（判据不放水）"
python3 - "$W/der.json" examples/m206_gcroot/BASELINE.tsv <<'PY' > "$W/align.log" 2>&1
import json, sys
from collections import Counter
d = json.load(open(sys.argv[1], encoding='utf-8'))
got = Counter((f['file'], f['func'], f['trigger'], ','.join(x['name'] for x in f['victims']))
              for f in d['findings'])
want = Counter()
for ln in open(sys.argv[2], encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln or ln.startswith('#'):
        continue
    c = ln.split('\t')
    if len(c) >= 5 and c[4] != '假阳':
        print("FAIL"); print("   BASELINE 里有非「假阳」判定：%s" % (c[0:4],)); sys.exit(1)
    want[tuple(c[0:4])] += 1
errs = []
for k, n in (got - want).items(): errs.append("新增候选（不在基线，疑似新缺陷）×%d: %s" % (n, k))
for k, n in (want - got).items(): errs.append("基线在册但扫描不到（判据失牙/已修）×%d: %s" % (n, k))
print("候选 %d · 基线在册 %d" % (sum(got.values()), sum(want.values())))
print("PASS" if not errs else "FAIL")
for e in errs: print("   " + e)
PY
if grep -q '^PASS$' "$W/align.log"; then
    ok "$(head -1 "$W/align.log")（逐条对齐 · 全部「假阳」且各带理由）"
else
    bad "候选/基线不符"; sed -n '2,8p' "$W/align.log" | sed 's/^/      /'
fi

step "⑤ 负控（3 道·各自独立判红·源逐字节还原）"
if [ "$NEG_SKIP" = 1 ]; then
    echo "  ⏭  负控跳过（CI 用 --neg-skip）"
else
    AUD=selfhost/gcroot_audit.py
    # A：规模锚点有牙 —— 抬高下限 ⇒ 必须**拒绝放行**（rc=3）而不是静默用空集
    python3 "$AUD" --min-triggers 100000 > "$W/nega.out" 2> "$W/nega.err"; RCA=$?
    if [ "$RCA" = 3 ] && grep -q '拒绝放行' "$W/nega.err"; then
        ok "负控 A 判红：规模锚点触发 rc=3 + 明确原因（**没有**静默回退）"
    else
        bad "负控 A 未判红（rc=$RCA）—— 派生失效会被当成全绿"; head -3 "$W/nega.err" | sed 's/^/      /'
    fi
    # B：A/B 对照有牙 —— 默认值改回 legacy ⇒ ③ 的对照失效
    if patch_one "$AUD" "default='derived'," "default='legacy'," NB; then
        python3 "$AUD" --json > "$W/negb.json" 2>/dev/null
        N=$(python3 -c "import json;print(len(json.load(open('$W/negb.json'))['findings']))" 2>/dev/null)
        if [ "${N:-x}" = 0 ]; then
            ok "负控 B 判红：默认值退回 legacy ⇒ 候选 0（证明 ③ 测的确实是「派生」）"
        else
            bad "负控 B 未判红（候选 ${N:-?}）"
        fi
    else
        bad "负控 B 打补丁失败"
    fi
    restore_all
    cmp -s "$AUD" "$SNAP/$AUD" || bad "负控 B 后审计器未还原"
    # C：缺陷 290 判据有牙 —— 关掉「登记动作 ⇒ 被登记对象的字段豁免」
    if patch_one "$AUD" "_rm = _REGISTER_RX.match(m.group(0))" "_rm = None" NC; then
        python3 "$AUD" --json > "$W/negc.json" 2>/dev/null
        N=$(python3 -c "import json;print(len(json.load(open('$W/negc.json'))['findings']))" 2>/dev/null)
        if [ "${N:-0}" -ge 4 ]; then
            ok "负控 C 判红：撤缺陷 290 判据 ⇒ 候选 $N（≥4，px_gen_lazy 族重新冒出来）"
        else
            bad "负控 C 未判红（候选 ${N:-?}，期望 ≥4）—— 该判据无牙"
        fi
    else
        bad "负控 C 打补丁失败"
    fi
    restore_all
    cmp -s "$AUD" "$SNAP/$AUD" || bad "负控 C 后审计器未还原"
fi

step "⑥ 覆盖边界（如实登记）"
echo '  ℹ️ 本门覆盖：**静态**（触发点/生产者派生 + 4 条候选的判定对齐）'
echo '  ℹ️ 动态取证已做：session_del（probe_rt.px 8 轮 × 40 请求压力档全绿）'
echo '  ℹ️ **未**动态覆盖：H3 族（h3_send_fields 的 body_val 修复只能靠静态判据 + 源码顺序；'
echo '     单机缺 QUIC listener 夹具）'
echo '  ℹ️ 判据缺口（M212 登记 → **M213 收口**）：① **跨函数**的「被调方登记了实参」'
echo '     ⇒ M213 缺陷 297 已判据化（**F1**，按位置配对 + 要求登记先于被调方第一个触发点）'
echo '     ⇒ h3_out_send / h3_srv_* 三条已从 BASELINE 移出（见 examples/m213_gcroot_precision/）；'
echo '     ② 「后置存活」（受害者读点在触发点之后）**仍未**判据化 ⇒ xml_build_node 需人工判定。'

echo
echo "M212 门：通过 $PASS · 失败 $FAIL"
if [ "$FAIL" -eq 0 ]; then echo "M212-VERIFY-OK"; exit 0; fi
echo "M212-VERIFY-FAIL"; exit 1
