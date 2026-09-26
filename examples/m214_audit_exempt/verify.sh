#!/usr/bin/env bash
# ============================================================
# M214（第 93 轮）门：**GC 根面审计器 —— 三条豁免规则的「有牙 / 不越界」**
# ------------------------------------------------------------
# 为什么有这个门：
#   M213 收尾时审计器还剩 **4 条候选**，全靠**人工判定**是假阳（理由写在代码注释里）：
#     · `bi_http_unix` 的错误支 `return px_net_err(…)` 里的 `headers`（提前返回不可达）
#     · `px_http_dispatch` 的 `vhandler` / `px_route_try_dispatch` 的 `handler`
#       （由**出参**交回，而写出是全局表元素 `g_vhosts[i].handler` / `g_routes[i].handler`）
#     · `xml_build_node` 的 `sv`（`bi_xml_escape` 的唯一分配点晚于全部实参读）
#   ⇒ 本轮把三条理由下沉为**判据**（R-A / R-B / R-C），使「候选 0」可复算、不靠人。
#
#   ⚠️ 本轮最贵的一课（写进门里当判据）：**首版 R-A 写成「触发点之后本函数内再无读」**
#     （后置死值）⇒ 一口吃掉 **6 条既有锚点**（hit5/hit6b/hit7/hit8/hit9/hit10），
#     而那些 fixture 正是**真形状**（本审计器的本分是「创建后必须登记」的**规则合规**）。
#     ⇒ 三条规则全部收紧为「**加一条可验证的条件才豁免**」。第 ①/⑤ 层守的就是这条线。
#
# 层：
#   ① 审计器自证 **25/25**（19 旧 + 6 新；其中 hit11/hit12/hit13 是三条规则的**反向判据**）
#   ② 真实审计 **候选 0**，且三类豁免计数**逐个非零**（R-A=1 · R-B=2 · R-C=1）
#   ③ A/B 三档各自独立：`--no-postdead`⇒1 · `--no-globalout`⇒2 · `--no-argread`⇒1 · 全关⇒4
#   ④ **判据回放（precision replay）**：撤掉 `px_route_try_dispatch` 的
#      `px_root_push_keep(params)` ⇒ 必须报出 `params@379`，而**同一个调用**上的
#      `handler`（全局根）**仍被豁免** —— 同语句、同调用、两个出参，一豁免一报出。
#   ⑤ 负控 3 道（各自独立判红 · 源逐字节还原）
#   ⑥ 覆盖边界（如实登记）
# 用法：bash examples/m214_audit_exempt/verify.sh [--neg-skip]
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="$(mktemp -d /tmp/m214_gate.XXXXXX)"
SNAP="$W/snap"
SRC_LIST="selfhost/gcroot_audit.py runtime/runtime_route.c"

snapshot()   { mkdir -p "$SNAP"; for f in $SRC_LIST; do mkdir -p "$SNAP/$(dirname "$f")"; cp "$f" "$SNAP/$f"; done; }
restore_all(){ for f in $SRC_LIST; do [ -f "$SNAP/$f" ] && cp "$SNAP/$f" "$f"; done; }
# ⚠️ 判据必须与**快照**比，不能拿 `git diff` 比 HEAD —— 本轮工作时源码本来就是
#   **未提交**状态（`gcroot_audit.py` 有 M214 的改动）⇒ 比 HEAD 会恒判「未还原」。
same_as_snap(){ for f in $SRC_LIST; do cmp -s "$SNAP/$f" "$f" || return 1; done; return 0; }
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad()  { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
step() { echo; echo "=== $1 ==="; }

patch_one() {   # $1=file $2=old $3=new $4=tag（**唯一**子串匹配，否则失败）
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

patch_line() {   # $1=file $2=行号 $3=新整行内容（按行号改 ⇒ 不怕子串重复）
    python3 - "$1" "$2" "$3" <<'PY' || return 1
import sys
p, ln, new = sys.argv[1], int(sys.argv[2]), sys.argv[3]
lines = open(p, encoding='utf-8').read().split('\n')
if not (1 <= ln <= len(lines)):
    print("  !! 行号 %d 越界（共 %d 行）" % (ln, len(lines))); sys.exit(1)
lines[ln - 1] = new
open(p, 'w', encoding='utf-8').write('\n'.join(lines))
PY
}

audit()  { python3 selfhost/gcroot_audit.py --trigger-set derived "$@" 2>/dev/null; }
cand_n() { audit "$@" | sed -n 's/.*\*\*候选 \([0-9]*\)\*\*.*/\1/p'; }

# ---------- ① 自证 ----------
step "① 审计器自证（25 锚点 = 13 必中 + 12 必不中）"
SELFTEST_FULL="$(python3 selfhost/gcroot_audit.py --self-test 2>&1)"
ST="$(printf '%s' "$SELFTEST_FULL" | tail -1)"
echo "  $ST"
if printf '%s' "$ST" | grep -q '^self-test: 25 通过 / 0 失败'; then
    ok "自证 25/25"
else
    bad "自证不是 25/25：$ST"
fi
for a in hit11 miss10 hit12 miss11 hit13 miss12; do
    if printf '%s' "$SELFTEST_FULL" | grep -q "^  $a .*✅"; then
        ok "锚点 $a 判绿"
    else
        bad "锚点 $a 未判绿"
    fi
done

# ---------- ② 候选 0 + 三类豁免计数 ----------
step "② 真实审计：候选 0，且三类豁免逐个非零"
SUM="$(audit | sed -n '/^扫描 /p')"
echo "  $SUM"
N="$(printf '%s' "$SUM"  | sed -n 's/.*\*\*候选 \([0-9]*\)\*\*.*/\1/p')"
RA="$(printf '%s' "$SUM" | sed -n 's/.*R-A 豁免 \([0-9]*\).*/\1/p')"
RB="$(printf '%s' "$SUM" | sed -n 's/.*R-B 豁免 \([0-9]*\).*/\1/p')"
RC="$(printf '%s' "$SUM" | sed -n 's/.*R-C 豁免 \([0-9]*\).*/\1/p')"
[ "$N" = "0" ]  && ok "候选 = 0"     || bad "候选 = $N（应为 0）"
[ "$RA" = "1" ] && ok "R-A 豁免 = 1" || bad "R-A 豁免 = $RA（应为 1）"
[ "$RB" = "2" ] && ok "R-B 豁免 = 2" || bad "R-B 豁免 = $RB（应为 2）"
[ "$RC" = "1" ] && ok "R-C 豁免 = 1" || bad "R-C 豁免 = $RC（应为 1）"

# ---------- ③ A/B 三档 ----------
step "③ A/B：每条规则**独立**生效（全关回到 4）"
c1="$(cand_n --no-postdead)";  [ "$c1" = "1" ] && ok "--no-postdead ⇒ 1"  || bad "--no-postdead ⇒ $c1（应 1）"
c2="$(cand_n --no-globalout)"; [ "$c2" = "2" ] && ok "--no-globalout ⇒ 2" || bad "--no-globalout ⇒ $c2（应 2）"
c3="$(cand_n --no-argread)";   [ "$c3" = "1" ] && ok "--no-argread ⇒ 1"   || bad "--no-argread ⇒ $c3（应 1）"
c4="$(cand_n --no-postdead --no-globalout --no-argread)"
[ "$c4" = "4" ] && ok "三档全关 ⇒ 4（= M213 的人工判定面）" || bad "三档全关 ⇒ $c4（应 4）"

# ---------- ④ 判据回放（precision replay） ----------
step "④ 判据回放：撤 params 的登记 ⇒ 必须报出 params，而同一调用的 handler 仍豁免"
KEEP_LN="$(grep -n '^    px_root_push_keep(params);$' runtime/runtime_route.c | head -1 | cut -d: -f1)"
if [ -n "$KEEP_LN" ] && patch_line runtime/runtime_route.c "$KEEP_LN" '    /* NEGCTL-M214-REPLAY */'; then
    echo "  （回放：撤销 runtime_route.c:$KEEP_LN 的 px_root_push_keep(params)）"
    OUT="$(audit --show-victims)"
    echo "$OUT" | grep -q 'runtime_route.c' \
        && ok "回放后 runtime_route.c 出现候选" || bad "回放后没有候选（规则把真缺陷吃掉了）"
    echo "$OUT" | grep -q 'params@379' \
        && ok "受害者点名 params@379（R-B 不豁免「本地构造」出参）" || bad "未点名 params@379"
    echo "$OUT" | grep -q 'handler@379' \
        && bad "handler 也被报出（R-B 失效 ⇒ 全局根也当成风险）" \
        || ok "同语句的 handler 仍被 R-B 豁免（精确性成立）"
    restore_all
    cmp -s "$SNAP/runtime/runtime_route.c" runtime/runtime_route.c \
        && ok "回放后源码逐字节还原（与快照 cmp 一致）" || bad "回放后源码未还原"
    AFTER="$(cand_n)"
    [ "$AFTER" = "0" ] && ok "还原后候选回到 0" || bad "还原后候选 = $AFTER（应 0）"
else
    bad "回放失败：找不到唯一的 \`    px_root_push_keep(params);\` 行"
fi

# ---------- ⑤ 负控 ----------
step "⑤ 负控（每道必须**独立**判红；源逐字节还原）"
if [ "$NEG_SKIP" = 1 ]; then
    echo "  ⏭ --neg-skip：跳过（CI 用；本机必须跑全）"
else
    # NC-A：三条规则**全部**关掉 ⇒ 第②层（候选须为 0）必红
    #   ⚠️ 锚点必须落在**生效处**：`POST_DEAD_ON` / `ARG_READ_FIRST_ON` 在 main() 里被
    #     `not args.no_xxx` **重新赋值** ⇒ 只改模块默认值**无效**（M209 的老坑，本轮又见：
    #     首版只改了模块默认 ⇒ 只关掉 R-B ⇒ 候选 2 而不是 4）。
    if patch_one selfhost/gcroot_audit.py 'GLOBAL_OUT_ON = True     # R-B' \
            'GLOBAL_OUT_ON = False    # NEGCTL-M214-A' a \
       && patch_one selfhost/gcroot_audit.py '    POST_DEAD_ON = not args.no_postdead' \
            '    POST_DEAD_ON = False   # NEGCTL-M214-A' a \
       && patch_one selfhost/gcroot_audit.py '    ARG_READ_FIRST_ON = not args.no_argread' \
            '    ARG_READ_FIRST_ON = False   # NEGCTL-M214-A' a; then
        n="$(cand_n)"
        [ "$n" = "4" ] && ok "NC-A 三规则全关 ⇒ 候选 4（第②层必红）✅" \
                       || bad "NC-A 三规则全关后候选 = $n（应 4）"
    else
        bad "NC-A 补丁锚点未命中"
    fi
    restore_all

    # NC-B（**判据自伤**）：把 R-B 的 glob 标记改成恒真（任何出参受害者都豁免）
    #   ⇒ 第④层的回放**不再报出 params** ⇒ 必红
    if patch_one selfhost/gcroot_audit.py \
            '                            _glob = bool(_gm.get(_names[_i]))' \
            '                            _glob = True   # NEGCTL-M214-B' b; then
        if [ -n "$KEEP_LN" ] && patch_line runtime/runtime_route.c "$KEEP_LN" '    /* NEGCTL-M214-REPLAY */'; then
            OUT2="$(audit --show-victims)"
            if echo "$OUT2" | grep -q 'params@379'; then
                bad "NC-B：放宽判据后仍报出 params ⇒ 该判据对本门无牙"
            else
                ok "NC-B glob 恒真 ⇒ params 不再被报出（第④层必红）✅"
            fi
            restore_all
        else
            bad "NC-B 回放锚点未命中"
        fi
    else
        bad "NC-B 补丁锚点未命中"
    fi
    restore_all

    # NC-C（**反向判据自伤**）：把 R-A 放宽成「凡 return 语句一律豁免受害者」
    #   ⇒ 锚点 hit11（return 表达式**含**受害者）必红 ⇒ 第①层必红
    if patch_one selfhost/gcroot_audit.py \
            "                        _rexpr = text[len('return'):]" \
            "                        _rexpr = ''   # NEGCTL-M214-C" c; then
        ST2="$(python3 selfhost/gcroot_audit.py --self-test 2>&1 | tail -1)"
        printf '%s' "$ST2" | grep -q '0 失败' \
            && bad "NC-C：放宽 R-A 后自证仍全绿 ⇒ 反向锚点 hit11 无牙" \
            || ok "NC-C 放宽 R-A ⇒ 自证判红（$ST2）⇒ 第①层必红 ✅"
    else
        bad "NC-C 补丁锚点未命中"
    fi
    restore_all

    same_as_snap && ok "三道负控后全部源文件逐字节还原（与快照 cmp 一致）" \
                 || bad "负控后源文件未还原"
fi

# ---------- ⑥ 覆盖边界 ----------
step "⑥ 覆盖边界（如实登记）"
echo "  · R-A/R-B/R-C 都是**窄条件豁免**（不是「凡不危及就豁免」）；本审计器的本分是"
echo "    **规则合规**（创建后必须登记），只接受「语义上确定不会再读 / 本就是 GC 根」的豁免。"
echo "  · **未判据化**（仍需人工，列为下一轮候选）："
echo "    - 「保守兜底·过近似」触发点（trigger_kind=conservative）仍参与候选；"
echo "    - 跨函数「被调方登记了实参」只覆盖**直接调用**（F1 表按位置配对）；"
echo "    - R-C 依赖「被调方在其**首次触发点**之前读完形参」这一**文本序**判据；"
echo '    - noreturn 集合目前由 __attribute__((noreturn)) 派生（实测只有 px_error）。'

echo
echo "═══ M214 门：通过 $PASS · 失败 $FAIL ═══"
[ "$FAIL" = 0 ] && echo "M214-VERIFY-OK" || echo "M214-VERIFY-FAIL"
exit $([ "$FAIL" = 0 ] && echo 0 || echo 1)
