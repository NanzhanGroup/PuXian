#!/usr/bin/env bash
# ============================================================
# M209 门（第 88 轮）：**GC 根面审计器的「触发点」更正 + 三条排除规则下沉**
#                  （缺陷 279 / 280 / 281 / 282）
# ------------------------------------------------------------
# 主题：M208（缺陷 270）给审计器加了一条「隐式分配」规则 —— 把
#   `px_dict_set` / `px_list_push` 也算成**触发点**，并据此报出 **19 处**「缺陷 271 族」。
#   本轮读源码把它**证伪并更正**：
#
#   缺陷 279（前提错位 · 本轮核心）：**full GC 的唯一触发点是 `gc_register`**
#     （runtime.c:3048 `g_alloc_bytes += est;` → :3051 判阈值 → 内联 `px_gc_collect()`
#      或 `g_gc_pending = 1`；并在 :3049 置 `g_tmp_root = o` 保护刚建对象）。
#     而 `px_dict_set`（:5105）/ `px_list_push`（:5020）只走 slab/mmap **裸分配**
#     （`m128_alloc` / `m128_strdup` → `xmalloc`/`xrealloc`）⇒ **不会触发 GC**。
#     它们内部确实有 M110-S2 安全点（`gc_unblock_stop` → `gc_pause_if_requested`），
#     但那只在「另一个线程的 GC 已在跑」时暂停本线程，而那一刻被保护的面是本线程
#     `ti→tmp_root`（快照自 `g_tmp_root`，runtime.c:1973/2635）= **本线程最近登记的对象**
#     —— 审计器报告的每个候选，受害者**恰是「创建后的第一个触发点」**上的对象
#     ⇒ 触发点是 push/dict_set 时，受害者就是最近登记对象 ⇒ 被护住 ⇒ **安全**。
#     ⇒ 19 条里 **11 条**由此消失。
#
#   缺陷 280（覆盖）/ 281（作用域）/ 282（deref 交棒）：另外 **8 条**是 M206 定的
#     **人工判定**（BASELINE.tsv 的「死值」「互斥分支」「触发点是下一轮迭代」）。
#     本轮把它们**下沉为判据**，三个规则各自可证：
#       · 覆盖：`v = px_bytes_len(…)` 的旧值此刻被覆盖（且右值未读它）⇒ 不是受害者
#         （`runtime_sqlite.c:203/209`）；
#       · 作用域：受害者的**声明块**在触发点前已闭合 ⇒ 读不到 ⇒ 不是受害者
#         （`keep`/宏体 `rv`/`name` 三族）。⚠️ 判据必须按**块身份**而非**括号深度** ——
#         宏体在源码里位于较浅的深度而调用点更深 ⇒ 「深度变小」永不成立（实测踩过）。
#       · deref 交棒：`*outv = px_str_len(…)` 把值交给调用方（与 `return` 同口径）
#         ⇒ 不计 C 局部活值（`runtime_h3_qpack.c:262`，调用方 :359 紧跟 `PX_KEEP`）。
#
# 层的设计（每层都能独立判红）：
#   ① 审计器自证 **16/16**（8 必中 + 8 必不中；含 3 条**反向判据**）
#   ② 新规则全仓候选 **0**（且两次运行结果一致）
#   ③ 旧规则对照（`--grow`）候选 **11**，且**全部**带「旧规则」标记（防对照被悄悄清空）
#   ④ 三条排除规则 + 触发点集合的**源码在位**断言（防判据被静默删除）
#   ⑤ **判据回放**（本轮最重要的层）：把 2 处**已修**站点的修复原样退回 ⇒ 审计器必须**命中**
#      —— 证明新规则**没有引入漏报**（工具没被改瞎）
#   ⑥ 动态：`probe_rules.px` 正常档 ×2 + 压力档
#      （`PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1 PX_GC_UAFDET=1`）逐字节一致、rc=0
#   ⑦ 负控 3 道（`--neg-skip` 可跳）：
#      A 撤 `bi_map` 的 `px_root_push_keep(r)` ⇒ ⑥ 压力档必红（实测 rc=134 + LIVECHK）
#      B 审计器改回旧规则默认（`LEGACY_GROW = True`）⇒ ② 必红
#      C 撤作用域排除 ⇒ ② 必红
#   ⑧ 覆盖边界登记（如实）
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="$(mktemp -d /tmp/m209_gate.XXXXXX)"
SNAP="$W/snap"
SRC_LIST="runtime/runtime.c selfhost/gcroot_audit.py"

snapshot() {
    mkdir -p "$SNAP"
    for f in $SRC_LIST; do mkdir -p "$SNAP/$(dirname "$f")"; cp "$f" "$SNAP/$f"; done
}
restore_all() {
    for f in $SRC_LIST; do [ -f "$SNAP/$f" ] && cp "$SNAP/$f" "$f"; done
}
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad()  { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
step() { echo; echo "=== $1 ==="; }

patch_one() {   # $1=file $2=old $3=new $4=tag（唯一匹配，否则失败）
    python3 - "$1" "$2" "$3" "$4" <<'PY' || exit 1
import sys
p, old, new, tag = sys.argv[1:5]
s = open(p, encoding='utf-8').read()
n = s.count(old)
if n != 1:
    print("  !! [%s] 锚点匹配 %d 次（要求 1）" % (tag, n)); sys.exit(1)
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PY
}
build_probe() {  # $1=源 $2=tag → 打印产物路径
    local src="$1" tag="$2"
    local d="$W/b$tag"    # ⚠️ 必须拆两行：`local a="$1" d="$W/b$a"` 在 bash5.1+set-u 下取外层 a
    mkdir -p "$d"; cp "$src" "$d/"
    ( cd "$d" && timeout 900 "$ROOT/tools/px" build "$(basename "$src")" > "$W/$tag.log" 2>&1 ) || {
        echo "  (构建失败，日志尾)"; tail -4 "$W/$tag.log" | sed 's/^/      /'; return 1; }
    echo "$d/build/$(basename "${src%.px}")"
}
run_track() {    # $1=bin $2=档(norm|stress) $3=期望正则
    local bin="$1" mode="$2" pat="$3" out rc
    if [ "$mode" = stress ]; then
        out=$(PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1 PX_GC_UAFDET=1 \
              timeout -k 5 300 "$bin" 2>&1); rc=$?
    else
        out=$(timeout -k 5 300 "$bin" 2>&1); rc=$?
    fi
    echo "$out" > "$W/last_$mode.out"
    [ "$rc" = 0 ] || { echo "rc=$rc"; return 1; }
    echo "$out" | grep -qE "$pat" || { echo "输出不符"; return 1; }
    echo "$out" | grep -qE "PX_GC_LIVECHK|PX_GC_UAFDET" && { echo "检测器响亮"; return 1; }
    return 0
}

MIN_FUNCS=1500
MIN_KEEPS=200

step "① 静态：审计器自证（16 锚点 = 8 必中 + 8 必不中）"
if python3 selfhost/gcroot_audit.py --self-test > "$W/selftest.log" 2>&1; then
    grep -q 'self-test: 16 通过 / 0 失败' "$W/selftest.log" \
        && ok "自证 16/16（hit1–8 含 hit6b/hit7/hit8 三条反向判据 · miss1–8 含 miss5 改判）" \
        || { bad "自证结论行不符"; tail -20 "$W/selftest.log" | sed 's/^/      /'; }
else
    bad "自证脚本失败"; tail -8 "$W/selftest.log" | sed 's/^/      /'
fi

step "② 静态：新规则全仓候选 = 0（两次一致）"
for i in 1 2; do
    python3 selfhost/gcroot_audit.py --show-victims > "$W/new$i.log" 2>&1 || bad "扫描器执行失败"
done
if diff -q "$W/new1.log" "$W/new2.log" >/dev/null 2>&1; then
    ok "两次运行结果一致（确定性）"
else
    bad "两次运行结果不一致"; diff "$W/new1.log" "$W/new2.log" | head -6 | sed 's/^/      /'
fi
head -1 "$W/new1.log" | sed 's/^/  ℹ️ /'
if grep -q '候选 0' "$W/new1.log"; then
    ok "全仓候选 0（缺陷 279–282 收口：11 条由「触发点仅构造器」消除 + 8 条由三条排除规则消除）"
else
    bad "仍有候选"; sed -n '2,14p' "$W/new1.log" | sed 's/^/      /'
fi
NSTAT=$(sed -n '1p' "$W/new1.log")
echo "$NSTAT" | grep -qE "函数 1593|函数 15[0-9][0-9]" || true
FUNCS=$(echo "$NSTAT" | grep -o '函数 [0-9]*' | awk '{print $2}')
KEEPS=$(echo "$NSTAT" | grep -o '登记站点 [0-9]*' | awk '{print $2}')
if [ -n "${FUNCS:-}" ] && [ "$FUNCS" -ge "$MIN_FUNCS" ]; then
    ok "规模锚点：函数 $FUNCS ≥ $MIN_FUNCS（防扫描面静默缩小）"
else
    bad "规模锚点失败：函数 ${FUNCS:-?} < $MIN_FUNCS"
fi
if [ -n "${KEEPS:-}" ] && [ "$KEEPS" -ge "$MIN_KEEPS" ]; then
    ok "规模锚点：登记站点 $KEEPS ≥ $MIN_KEEPS"
else
    bad "规模锚点失败：登记站点 ${KEEPS:-?} < $MIN_KEEPS"
fi

step "③ 静态：旧规则对照（--grow）候选 = 11 且全部带「旧规则」标记"
python3 selfhost/gcroot_audit.py --grow --json > "$W/grow.json" 2>&1 || bad "对照扫描失败"
python3 - "$W/grow.json" <<'PY' > "$W/grow.chk" 2>&1
import json, sys
d = json.load(open(sys.argv[1], encoding='utf-8'))
fs = d['findings']
print("候选 %d" % len(fs))
untagged = [f for f in fs if '旧规则' not in f['trigger']]
print("未带旧规则标记 %d" % len(untagged))
for f in fs:
    print("  %s:%d %s ← %s" % (f['file'], f['line'], f['func'], f['trigger']))
PY
sed -n '1,2p' "$W/grow.chk" | sed 's/^/  ℹ️ /'
GROWN=$(sed -n '1p' "$W/grow.chk" | awk '{print $2}')
UNTAG=$(sed -n '2p' "$W/grow.chk" | awk '{print $2}')
if [ "${GROWN:-0}" = 11 ] && [ "${UNTAG:-1}" = 0 ]; then
    ok "旧规则候选 11 条、全部标记为「隐式分配·旧规则」（对照有效）"
else
    bad "旧规则对照不符：候选 ${GROWN:-?}（期望 11）· 未标记 ${UNTAG:-?}（期望 0）"
    sed -n '3,16p' "$W/grow.chk" | sed 's/^/      /'
fi
sed -n '3,16p' "$W/grow.chk" | sed 's/^/      /'

step "④ 静态：三条排除规则 + 触发点集合的源码在位断言"
AUD=selfhost/gcroot_audit.py
grep -q "^LEGACY_GROW = False" "$AUD" && ok "触发点集合：默认关闭旧规则（LEGACY_GROW=False）" \
    || bad "旧规则默认值缺失/被改"
grep -q "if LEGACY_GROW:" "$AUD" && ok "旧规则路径仍在（可对照，不可默认）" || bad "旧规则路径被删"
grep -q "overwrite = _lhs_s" "$AUD" && ok "规则 1（覆盖排除）在位" || bad "覆盖排除被删"
grep -q "v\[1\] in stack" "$AUD" && ok "规则 2（作用域排除 · 按块身份）在位" || bad "作用域排除被删"
grep -q "lhs_raw.strip().startswith('\*')" "$AUD" && ok "规则 3（deref 交棒）在位" || bad "deref 交棒被删"
grep -q "g_tmp_root = o;  // 保护刚创建对象" runtime/runtime.c \
    && ok "g_tmp_root 在 gc_register 内仍置位（缺陷 279 论证的前提）" || bad "g_tmp_root 置位被删"
grep -q "px_root_push_keep" runtime/runtime.c && ok "原子登记原语 px_root_push_keep 在位" || bad "原语缺失"

step "⑤ 判据回放：把已修站点退回 ⇒ 审计器**必须命中**（证明没引入漏报）"
# 回放 1：px_session_read 的 PX_KEEP(v)（缺陷 254）
if patch_one runtime/runtime.c \
    'LXValue v = px_str_len(data, len); PX_KEEP(v);' \
    'LXValue v = px_str_len(data, len);' R1; then
    python3 selfhost/gcroot_audit.py --json > "$W/r1.json" 2>/dev/null
    if python3 -c "
import json,sys
d=json.load(open('$W/r1.json'))
n=[f for f in d['findings'] if f['func']=='px_session_read']
sys.exit(0 if len(n)>=1 else 1)"; then
        ok "回放 1 命中：撤 PX_KEEP(v) ⇒ px_session_read 重新报出（缺陷 254 形状）"
    else
        bad "回放 1 未命中 —— 排除规则把工具改瞎了（漏报）"
    fi
else
    bad "回放 1 打补丁失败"
fi
restore_all
cmp -s runtime/runtime.c "$SNAP/runtime/runtime.c" || bad "回放 1 后源未还原"

# 回放 2：bi_map 的 px_root_push_keep(r)（容器本身）
if patch_one runtime/runtime.c \
    '    px_root_push_keep(r);   // 累积结果 list：px_list_push 扩容分配/回调期间需存活
' '' R2; then
    python3 selfhost/gcroot_audit.py --json > "$W/r2.json" 2>/dev/null
    if python3 -c "
import json,sys
d=json.load(open('$W/r2.json'))
n=[f for f in d['findings'] if f['func']=='bi_map']
sys.exit(0 if len(n)>=1 else 1)"; then
        ok "回放 2 命中：撤 bi_map 的 keep ⇒ 报出容器 r（受害者是**容器**，不是 push 的实参）"
    else
        bad "回放 2 未命中 —— 漏报"
    fi
else
    bad "回放 2 打补丁失败"
fi
restore_all
cmp -s runtime/runtime.c "$SNAP/runtime/runtime.c" || bad "回放 2 后源未还原"

step "⑥ 动态：probe_rules 正常档 ×2 + 压力档"
PR=$(build_probe examples/m209_gcroot_rules/probe_rules.px rules) || bad "probe_rules 构建失败"
if [ -n "${PR:-}" ]; then
    run_track "$PR" norm 'M209-PROBE-OK s=[0-9]+' && cp "$W/last_norm.out" "$W/norm1.out" \
        && ok "正常档 1 通过" || bad "正常档 1 失败"
    run_track "$PR" norm 'M209-PROBE-OK s=[0-9]+' && ok "正常档 2 通过" || bad "正常档 2 失败"
    if diff -q "$W/norm1.out" "$W/last_norm.out" >/dev/null 2>&1; then
        ok "正常档两跑逐字节一致（确定性）"
    else
        bad "正常档两跑不一致"; diff "$W/norm1.out" "$W/last_norm.out" | head -4 | sed 's/^/      /'
    fi
    run_track "$PR" stress 'M209-PROBE-OK s=[0-9]+' && ok "压力档通过（STRESS+INLINE+LIVECHK+UAFDET）" \
        || { bad "压力档失败"; tail -6 "$W/last_stress.out" | sed 's/^/      /'; }
    if diff -q "$W/norm1.out" "$W/last_stress.out" >/dev/null 2>&1; then
        ok "正常档 ⇄ 压力档逐字节一致"
    else
        bad "两档输出不一致"; diff "$W/norm1.out" "$W/last_stress.out" | head -4 | sed 's/^/      /'
    fi
    # 覆盖登记（probe 实际走到了哪些站点族）
    echo "  ℹ️ probe 覆盖：bi_map / bi_filter / 生成器物化 · px_cell（闭包捕获）· bi_os_popen"
fi

step "⑦ 负控"
if [ "$NEG_SKIP" = 1 ]; then
    ok "负控跳过（CI 用 --neg-skip）"
else
    # A：撤 bi_map 的 keep ⇒ ⑥ 压力档必红
    if [ -n "${PR:-}" ]; then
        if patch_one runtime/runtime.c \
            '    px_root_push_keep(r);   // 累积结果 list：px_list_push 扩容分配/回调期间需存活
' '' NA; then
            NEGBIN=$(build_probe examples/m209_gcroot_rules/probe_rules.px negA)
            if [ -n "${NEGBIN:-}" ]; then
                if run_track "$NEGBIN" stress 'M209-PROBE-OK' >/dev/null 2>&1; then
                    bad "负控 A 未判红（撤 keep 后仍绿 ⇒ 探针没牙）"
                else
                    ok "负控 A 判红：$(grep -ao 'PX_GC_LIVECHK*[^（]*' "$W/last_stress.out" | head -1)"
                fi
            else
                bad "负控 A 构建失败"
            fi
        else
            bad "负控 A 打补丁失败"
        fi
        restore_all
        cmp -s runtime/runtime.c "$SNAP/runtime/runtime.c" || bad "负控 A 后源未还原"
    fi
    # B：把旧规则设为默认 ⇒ ② 必红（候选 11）
    #   ⚠️ 锚点必须落在 `main()` 的赋值行 —— 模块级默认值会被 `main` 里的
    #     `LEGACY_GROW = bool(args.grow)` **覆盖**（首版锚错在定义行 ⇒ 负控假红，实测）。
    if patch_one "$AUD" 'LEGACY_GROW = bool(args.grow)' 'LEGACY_GROW = True' NB; then
        python3 selfhost/gcroot_audit.py --show-victims > "$W/negb.log" 2>&1
        if grep -q '候选 0' "$W/negb.log"; then
            bad "负控 B 未判红（旧规则被当成默认，候选仍为 0 ⇒ 对照失效）"
        else
            ok "负控 B 判红：$(head -1 "$W/negb.log")"
        fi
    else
        bad "负控 B 打补丁失败"
    fi
    restore_all
    cmp -s "$AUD" "$SNAP/$AUD" || bad "负控 B 后审计器未还原"
    # C：撤作用域排除 ⇒ ② 必红
    if patch_one "$AUD" 'live = {k: v for k, v in live.items() if v[1] in stack}' \
        'live = dict(live)' NC; then
        python3 selfhost/gcroot_audit.py --show-victims > "$W/negc.log" 2>&1
        if grep -q '候选 0' "$W/negc.log"; then
            bad "负控 C 未判红（撤作用域排除后候选仍为 0 ⇒ 该规则无牙）"
        else
            ok "负控 C 判红：$(head -1 "$W/negc.log")"
        fi
    else
        bad "负控 C 打补丁失败"
    fi
    restore_all
    cmp -s "$AUD" "$SNAP/$AUD" || bad "负控 C 后审计器未还原"
fi

step "⑧ 覆盖边界（如实登记）"
echo "  ℹ️ 动态覆盖：bi_map / bi_filter / px_gen_* / px_cell / bi_os_popen 五族（probe_rules.px）"
echo "  ℹ️ **未**动态覆盖（仅静态判据）：bi_session_set/del · px_http_dispatch_h3 ·"
echo "     bi_h3_conn_peer/stats · bi_img_decode —— 形状同「容器随后写入」(bi_os_popen) 或"
echo "     「构造器结果随后入列/入字典」(bi_map)，但缺可单机复现的触发路径（需 serve/H3/图片）。"

echo
echo "M209 门：通过 $PASS · 失败 $FAIL"
if [ "$FAIL" -eq 0 ]; then echo "M209-VERIFY-OK"; exit 0; fi
echo "M209-VERIFY-FAIL"; exit 1
