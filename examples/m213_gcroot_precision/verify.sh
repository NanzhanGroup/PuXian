#!/usr/bin/env bash
# ============================================================
# M213（第 92 轮）门：**GC 根面审计器 —— 判据的「诚实度」与「覆盖面」**
# ------------------------------------------------------------
# 为什么有这个门：
#   M209 把审计器的触发点定义成「可达 `gc_register`」后，「全仓候选 0」被当成干净。
#   M212 把触发点改为**源码派生**、照出候选 3；本轮（M213）继续追同一件事的两个面：
#     · **覆盖面**（漏报）：出口参数式构造函数（`px_as_list(v,&out)`）、成员取址别名、
#       **显式 GC 入口**（`px_gc_collect`/`px_gc_poll`/`bi_gc` —— 不经过 `gc_register`）
#       —— 三处整族看不见；
#     · **诚实度**（假阳/不可预测）：间接调用判据实现成「括号后跟星号」这一**语法形状**，
#       实测 99 个假种子、闭包虚增 169 个函数、并产出候选 #3。
#
# 层：
#   ① 派生器自证 19/19（含间接调用**分层**双向、显式 GC 族、F1 正/反）
#   ② 审计器自证 19/19
#   ③ **分诊**：`trigger_kind` 能把「保守兜底」与「精确」分开（且 tight 档确实少掉它）
#   ④ **覆盖面**：显式 GC 族在 **loose 与 tight 两档都在** TRIGGER（= 「不能收紧」的实证）
#   ⑤ **F1 有牙**：默认候选 4 ⇄ `--no-callee-keep` 候选 7（差 = 被 F1 豁免的 3 条）
#   ⑥ 4 条候选 ⇄ `examples/m206_gcroot/BASELINE.tsv` §① 逐条一致
#   ⑦ **push/pop 平衡**（静态）：`px_route_try_dispatch` 的登记**前移**后，
#      `px_root_push_keep(params)` = 1 且 `px_root_pop()` 数 = return 路径数
#   ⑧ 负控 3 道（各自独立判红 · 源逐字节还原）
#   ⑨ 覆盖边界（如实登记）
# 用法：bash examples/m213_gcroot_precision/verify.sh [--neg-skip]
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="$(mktemp -d /tmp/m213_gate.XXXXXX)"
SNAP="$W/snap"
SRC_LIST="selfhost/gcroot_audit.py selfhost/gcroot_derive.py runtime/runtime.c runtime/runtime_route.c"

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
count_cand() { python3 -c "import json,sys;print(len(json.load(open(sys.argv[1]))['findings']))" "$1" 2>/dev/null; }

step "① 派生器自证（gcroot_derive.py · 19 锚点）"
if python3 selfhost/gcroot_derive.py --self-test > "$W/derive.log" 2>&1; then
    grep -q 'self-test: 19 通过 / 0 失败' "$W/derive.log" \
        && ok "派生器自证 19/19（含 ⑫/⑮ 间接调用分层**双向**、⑭ 显式 GC 族、⑯/⑰ F1 正/反）" \
        || { bad "自证结论行不符"; tail -12 "$W/derive.log" | sed 's/^/      /'; }
else
    bad "派生器自证失败"; tail -12 "$W/derive.log" | sed 's/^/      /'
fi

step "② 审计器自证（gcroot_audit.py · 19 锚点）"
if python3 selfhost/gcroot_audit.py --self-test > "$W/selftest.log" 2>&1; then
    grep -q 'self-test: 19 通过 / 0 失败' "$W/selftest.log" \
        && ok "审计器自证 19/19（含 hit9/hit10/miss9：出口参数式构造器 + 成员取址别名）" \
        || { bad "自证结论行不符"; tail -8 "$W/selftest.log" | sed 's/^/      /'; }
else
    bad "审计器自证失败"; tail -8 "$W/selftest.log" | sed 's/^/      /'
fi

step "③ 分诊：trigger_kind 把「保守兜底」与「精确」分开 + tight 档确实少掉它"
python3 selfhost/gcroot_audit.py --json > "$W/loose.json" 2> "$W/loose.err" || bad "loose 扫描失败"
python3 selfhost/gcroot_audit.py --json --indirect tight > "$W/tight.json" 2>/dev/null || bad "tight 扫描失败"
python3 - "$W/loose.json" "$W/tight.json" <<'PY' > "$W/triage.log" 2>&1
import json, sys
lo = json.load(open(sys.argv[1], encoding='utf-8'))['findings']
ti = json.load(open(sys.argv[2], encoding='utf-8'))['findings']
errs = []
kinds = {}
for f in lo:
    kinds.setdefault(f.get('trigger_kind'), []).append(f)
cons = [f for f in lo if f.get('trigger_kind') == 'conservative']
if not cons:
    errs.append("loose 档**没有任何** conservative 候选 ⇒ 分诊失效或过近似已消失")
else:
    for f in cons:
        if 'px_rate_limit_try' not in f['trigger']:
            errs.append("conservative 候选出乎意料：%s" % f['trigger'])
# tight 档下该触发点必须消失（换成别的精确触发点 ⇒ 触发点名字必须变）
lt = [f for f in lo if 'px_rate_limit_try' in f['trigger']]
tt = [f for f in ti if 'px_rate_limit_try' in f['trigger']]
if lt and tt:
    errs.append("tight 档仍报 px_rate_limit_try ⇒ 档位没生效（过近似未被排除）")
print("loose 候选 %d（precise %d / conservative %d）· tight 候选 %d"
      % (len(lo), len(kinds.get('precise') or []), len(cons), len(ti)))
print("PASS" if not errs else "FAIL")
for e in errs:
    print("   " + e)
PY
if grep -q '^PASS$' "$W/triage.log"; then ok "$(head -1 "$W/triage.log")（分诊可用：过近似代价一眼可见）"
else bad "分诊不符"; sed -n '2,6p' "$W/triage.log" | sed 's/^/      /'; fi

step "④ 覆盖面：显式 GC 族（缺陷 296）在 **loose 与 tight 两档都在** TRIGGER"
python3 - "$W/loose.json" "$W/tight.json" <<'PY' > "$W/gcseed.log" 2>&1
import sys
sys.path.insert(0, '/data/code/puxian/selfhost')
import gcroot_derive as d
ROOT = '/data/code/puxian'
lt = d.derive_sets(ROOT, 'loose')[0]
tt = d.derive_sets(ROOT, 'tight')[0]
errs = []
for n in ('px_gc_collect', 'px_gc_poll', 'bi_gc'):
    if n not in lt:
        errs.append("%s ∉ loose TRIGGER（显式 GC 入口族丢失）" % n)
    if n not in tt:
        errs.append("%s ∉ tight TRIGGER（**收紧即新造漏报** —— 这正是「不能收紧」的实证）" % n)
print("loose T=%d · tight T=%d · 显式入口族 %s"
      % (len(lt), len(tt), '齐' if not errs else '缺'))
print("PASS" if not errs else "FAIL")
for e in errs:
    print("   " + e)
PY
if grep -q '^PASS$' "$W/gcseed.log"; then ok "$(head -1 "$W/gcseed.log")"
else bad "显式 GC 族覆盖面不符"; sed -n '2,6p' "$W/gcseed.log" | sed 's/^/      /'; fi

step "⑤ F1 有牙（A/B）：默认候选 4 ⇄ --no-callee-keep 候选 7"
python3 selfhost/gcroot_audit.py --json --no-callee-keep > "$W/nof1.json" 2> "$W/nof1.err" || bad "F1 关闭扫描失败"
N_ON=$(count_cand "$W/loose.json"); N_OFF=$(count_cand "$W/nof1.json")
[ "${N_ON:-x}" = 4 ] && ok "默认（F1 开）候选 4" || bad "默认候选 ${N_ON:-?}（期望 4）"
[ "${N_OFF:-x}" = 7 ] && ok "F1 关 ⇒ 候选 7（多出 3 条 = 被 F1 豁免的 h3_send_fields/h3_fields_to_request×2）" \
                      || bad "F1 关候选 ${N_OFF:-?}（期望 7）—— 规则无牙"
grep -q 'F1 表（被调方入口已登记形参）= 7 条' "$W/loose.err" \
    && ok "stderr 打印 F1 表规模（7 条；缺它=规则静默失效）" \
    || bad "未打印 F1 表规模诊断行"

step "⑥ 候选 4 ⇄ m206 BASELINE §① 逐条一致（判据不放水）"
python3 - "$W/loose.json" examples/m206_gcroot/BASELINE.tsv <<'PY' > "$W/align.log" 2>&1
import json, sys
from collections import Counter
got = Counter((f['file'], f['func'], f['trigger'],
               ','.join(x['name'] for x in f['victims']))
              for f in json.load(open(sys.argv[1], encoding='utf-8'))['findings'])
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
for k, n in (got - want).items():
    errs.append("新增候选（不在基线，疑似新缺陷）×%d: %s" % (n, k))
for k, n in (want - got).items():
    errs.append("基线在册但扫描不到（判据失牙/已修）×%d: %s" % (n, k))
print("候选 %d · 基线在册 %d" % (sum(got.values()), sum(want.values())))
print("PASS" if not errs else "FAIL")
for e in errs:
    print("   " + e)
PY
if grep -q '^PASS$' "$W/align.log"; then ok "$(head -1 "$W/align.log")"
else bad "候选/基线不符"; sed -n '2,8p' "$W/align.log" | sed 's/^/      /'; fi

step "⑦ push/pop 平衡（缺陷 298 登记前移后 · 静态）"
python3 - <<'PY' > "$W/balance.log" 2>&1
import importlib.util
import re
import sys

# ⚠️ 必须用**仓库自己的函数扫描器**取函数体（按花括号深度切、且已剥注释/字面量）：
#   首版手写「从签名后的第一个 { 数到深度归零」——**注释/字符串里的花括号**会破坏计数，
#   实测把两个函数体连在一起（push 数成 2、return 数成 8）⇒ 判据自己先错。
spec = importlib.util.spec_from_file_location('d', '/data/code/puxian/selfhost/gcroot_derive.py')
d = importlib.util.module_from_spec(spec)
spec.loader.exec_module(d)
funcs = d._scan('/data/code/puxian')[0]
body = funcs['px_route_try_dispatch'][3]
errs = []
push = len(re.findall(r'px_root_push_keep\(params\)', body))
pops = len(re.findall(r'px_root_pop\(\)', body))
i_push = body.find('px_root_push_keep(params)')
rets_after = len(re.findall(r'\breturn\b', body[i_push:])) if i_push >= 0 else 0
i_rm = body.find('route_match(')
i_rate = body.find('px_rate_limit_try(')
if push != 1:
    errs.append("px_root_push_keep(params) 出现 %d 次（期望 1 —— 登记点应唯一且已前移）" % push)
if pops != rets_after:
    errs.append("登记点之后 px_root_pop() %d 次 ≠ return %d 次 ⇒ **push/pop 不平衡**"
                "（泄漏根栈条目 = 每次请求多一条）" % (pops, rets_after))
if not (i_rm >= 0 and 0 <= i_push < i_rate):
    errs.append("登记点未落在 `route_match` 之后、`px_rate_limit_try` 之前 ⇒ 缺陷 298 未生效")
print("push=%d pop=%d（登记点后 return=%d）" % (push, pops, rets_after))
print("PASS" if not errs else "FAIL")
for e in errs:
    print("   " + e)
PY
if grep -q '^PASS$' "$W/balance.log"; then ok "$(head -1 "$W/balance.log")（每个出口各配对一次）"
else bad "push/pop 不平衡"; sed -n '2,6p' "$W/balance.log" | sed 's/^/      /'; fi

step "⑧ 负控（3 道·各自独立判红·源逐字节还原）"
if [ "$NEG_SKIP" = 1 ]; then
    echo "  ⏭  负控跳过（CI 用 --neg-skip）"
else
    AUD=selfhost/gcroot_audit.py
    DER=selfhost/gcroot_derive.py

    # A：F1 规则有牙 —— 关掉 ⇒ 候选 7 ≠ 4 ⇒ ⑤ 与 ⑥ 必须判红
    restore_all
    python3 "$AUD" --json --no-callee-keep > "$W/negA.json" 2>/dev/null
    if [ "$(count_cand "$W/negA.json")" != 4 ]; then
        ok "负控 A 判红：撤 F1 ⇒ 候选 $(count_cand "$W/negA.json")（≠4）"
    else
        bad "负控 A 未判红（撤 F1 后候选仍 4）⇒ 规则无牙"
    fi
    restore_all; cmp -s "$AUD" "$SNAP/$AUD" || bad "负控 A 后审计器未还原"

    # B：缺陷 296 有牙 —— 把「显式 GC 入口」两处口子都堵上 ⇒
    #    显式入口族必须**掉出** TRIGGER（证明 ④ 测的就是它）
    #    ⚠️ 锚点选**不含反斜杠**的片段：首版把整条 `re.compile(...)` 当锚点，
    #       补丁串里的转义被 shell/python 两层吃掉 ⇒ 生成**坏正则** ⇒ 负控自己报错
    #       （「正则未闭合」），而门只看到「未判红」—— 又一次「判据失败信息指不到真因」。
    restore_all
    if patch_one "$DER" "(gc_register|px_gc_collect|px_gc_poll)" "(gc_register|zzz_none_a|zzz_none_b)" B \
       && patch_one "$DER" "_EXPLICIT_GC = ('px_gc_collect', 'px_gc_poll')" "_EXPLICIT_GC = ()" B2; then
        N=$(python3 - "$DER" <<'PY'
import sys, importlib.util
spec = importlib.util.spec_from_file_location('d', sys.argv[1])
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
# ⚠️ 必须看 **tight** 档：显式 GC 族在 loose 档**另有**宽松兜底那条路
#    （`px_gc_collect` 体内有 `sizeof(*x)` 形状）—— 这正是缺陷 296 的叙事：
#    「这一族**只**靠宽松兜底偶然覆盖 ⇒ 一旦收紧就整族漏报」。
#    所以「堵掉显式入口 ⇒ 它们应掉出」只能在 tight 档观察。
t = m.derive_sets('/data/code/puxian', 'tight')[0]
print(len([n for n in ('px_gc_collect', 'px_gc_poll', 'bi_gc') if n in t]))
PY
)
        if [ "${N:-x}" = 0 ]; then
            ok "负控 B 判红：堵掉显式 GC 入口 ⇒ **tight** 档三个入口族全部掉出 TRIGGER（④ 有牙）"
        else
            bad "负控 B 未判红（tight 档仍剩 ${N} 个）"
        fi
    else
        bad "负控 B 打补丁失败"
    fi
    restore_all
    cmp -s "$DER" "$SNAP/$DER" || bad "负控 B 后派生器未还原"

    # C：缺陷 293 有牙 —— 撤 bi_session_del 的登记 ⇒ 候选必须回到 5（多出 session_del）
    restore_all
    if patch_one runtime/runtime.c "    px_root_push();
    LXValue sess = px_session_read(g_cur_sid);
    PX_KEEP(sess);
    LXValue nd = px_dict();
    PX_KEEP(nd);" "    LXValue sess = px_session_read(g_cur_sid);
    LXValue nd = px_dict();" C; then
        python3 "$AUD" --json > "$W/negC.json" 2>/dev/null
        NC=$(count_cand "$W/negC.json")
        if [ "${NC:-0}" -ge 5 ] && grep -q 'bi_session_del' "$W/negC.json"; then
            ok "负控 C 判红：撤 bi_session_del 的登记 ⇒ 候选 ${NC}（≥5，session_del 重新冒出来）"
        else
            bad "负控 C 未判红（候选 ${NC:-?}）⇒ 缺陷 293 的修复无牙"
        fi
    else
        bad "负控 C 打补丁失败"
    fi
    restore_all
    cmp -s runtime/runtime.c "$SNAP/runtime/runtime.c" || bad "负控 C 后 runtime.c 未还原"
fi

step "⑨ 覆盖边界（如实登记）"
echo '  ℹ️ 本门覆盖：**静态**（派生两面 · 分诊 · 覆盖面 · 候选⇄基线 · push/pop 平衡）'
echo '  ℹ️ 动态覆盖**不重复**：HTTP/session 面在 `examples/m206_gcroot/`（两档 40 请求）；'
echo '     route 面（缺陷 298 改的就是它）在 `examples/m173_http_proxy/`（M211 起压力档 PASS）'
echo '     —— 二者都在 `selfhost/m116_gates.sh` 全量门里跑。'
echo '  ℹ️ **未**判据化（下一轮候选）：① 「全局表持有」（`g_routes[i].handler` / `g_vhosts[].handler`）'
echo '     ⇒ 候选 #2/#3 仍需人工判定；② 「后置存活」（受害者读点在触发点之后）⇒ 候选 #1/#4。'
echo '  ℹ️ 间接调用档位默认 `loose`（**有意过近似**，宁多收）：实测收紧会让 160 个函数离开'
echo '     TRIGGER，其中含显式 GC 族 ⇒ **会新造漏报**，故只作分诊用。'

echo
echo "M213 门：通过 $PASS · 失败 $FAIL"
if [ "$FAIL" -eq 0 ]; then echo "M213-VERIFY-OK"; exit 0; fi
echo "M213-VERIFY-FAIL"; exit 1
