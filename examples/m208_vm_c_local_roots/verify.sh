#!/usr/bin/env bash
# ============================================================
# M208 门（第 87 轮）：**VM 解释循环内的「构造 → 登记」窗口**（缺陷 268 / 269 / 270）
# ------------------------------------------------------------
# 主题：M207 的压力筛把 examples/m88_s3/s1b_gc_stress、m93_s3/coro_gc_block、m96_s3
#   判为 FAIL（LIVECHK 响亮 + SIGABRT/core），当时记账为「缺陷 267 家族（帧槽根面）」。
#   本轮用「引用溯源 + 执行帧指令 + 槽台账」三件诊断把它定位到**真病灶**：
#
#   缺陷 268（站点）：`PXOP_NEWDICT` / `px_list_n` 把新建容器只放在 **C 局部**，
#     而紧接着的 `px_dict_set` / `px_list_push` **本身会分配**（键副本 m128_strdup、
#     条目数组 xrealloc 扩容）⇒ 容器在「落槽/登记」之前就可能被回收。
#   缺陷 269（**真根因 · 影响面远超上述两站点**）：`px_gc_collect` 在出口把
#     `g_tmp_root = NULL`。而 `g_tmp_root` 正是「构造函数刚建好、调用方尚未接住」
#     这段窗口的**唯一**保护；更糟的是这条路上恰有一个 M110-S2 **协作式安全点**
#     （`gc_unblock_stop → gc_pause_if_requested`）⇒ 本线程会**主动**停在
#     「对象已在堆上、却不在任何根面」的状态，被并发的另一轮 GC 收走。
#     ⇒ 这就是 M170/182/183/206/207 的所有 `PX_KEEP` 修复所依赖的前提：
#       **前提被回收出口自己打破了**。
#     修法：出口**不清** `g_tmp_root`（语义改为「本线程最近登记的对象，构造窗口未结束」
#        —— 每轮都会标记它 ⇒ 不可能被回收；代价 = 每线程最多多保活 1 个对象）。
#   缺陷 270（判据）：静态审计器**没把 `px_dict_set`/`px_list_push` 算作分配点**
#     ⇒ 「先建容器、再往里放东西」这一族**整族静默漏报**（NEWDICT 就在其中）。
#
# 判据（M208 定稿）：
#   S13 **构造与登记之间不得存在安全点** ⇒ 惯用法从两段式
#       `px_root_push(); PX_KEEP(x);`（两次 gc_unblock_stop，中间即窗口）
#       改为**原子** `px_root_push_keep(x)`（一个临界区）；本门断言「相邻两段式 = 0」。
#   S14 **隐式分配**（`px_dict_set`/`px_list_push`…）计入审计器的分配点（`--grow`）。
#
# 层：
#  ① 静态：审计器自证 10/10 + `--grow` 下 runtime/vm.c **候选 0**（缺陷 268 收口）
#  ② 静态：S13 不变量 —— runtime/*.c + runtime/vm.c 里**相邻** `px_root_push();`+`PX_KEEP(`
#     出现 0 次；且 `px_root_push_keep` 的实现在位（含 LIVECHK 自检）
#  ③ 静态：`g_tmp_root` 在 px_gc_collect **两条路径**都不得置 NULL（缺陷 269 的守卫）
#  ④ 动态：probe_lit（单线程）/ probe_coro（8 协程）两档 —— 正常档与
#     压力档（PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1）都必须通过且结果一致
#  ⑤ 动态回归：m93_s3/coro_gc_block 压力档（修前 **必红**：LIVECHK + SIGABRT）
#  ⑥ 负控 3 道（各自独立判红 + 源逐字节还原）：
#     A 恢复 `g_tmp_root = NULL;`（并发路径）⇒ ④ 压力档必红（缺陷 269 复现）
#     B 撤 `PXOP_NEWDICT` 的 `px_root_push_keep(d)` ⇒ ④ 压力档必红（缺陷 268 复现）
#     C 撤 `px_list_n` 的注册 ⇒ ④ 压力档必红（缺陷 268 同族）
#  覆盖边界（如实）：`--grow` 规则另照出 **18 处既有站点**（缺陷 271 族，见
#     docs/GC_ROOTS.md §8）—— 本轮**只登记不修**，故本门对全仓 `--grow` 候选数
#     **不做断言**（只打印记录），仅对 `runtime/vm.c` 断言 0。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="$(mktemp -d /tmp/m208_gate.XXXXXX)"
SNAP="$W/snap"
SRC_LIST="runtime/runtime.c runtime/vm.c runtime/coro.c"
snapshot() {
    mkdir -p "$SNAP"
    for f in $SRC_LIST; do mkdir -p "$SNAP/$(dirname "$f")"; cp "$f" "$SNAP/$f"; done
}
restore_all() {
    for f in $SRC_LIST; do [ -f "$SNAP/$f" ] && cp "$SNAP/$f" "$f"; done
}
# M191 纪律：负控各自独立、干净起点（M208 复核：先 restore 再 snapshot）
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad()  { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
step() { echo; echo "=== $1 ==="; }

patch_one() {   # $1=file $2=old $3=new $4=tag
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
    local d="$W/b$tag"     # ⚠️ M208：**必须**拆两行 —— `local a="$1" d="$W/b$a"` 在
                           #   bash 5.1 + set -u 下 `d` 取的是**外层** `a`（实测 unbound）
    mkdir -p "$d"; cp "$src" "$d/"
    ( cd "$d" && timeout 900 "$ROOT/tools/px" build "$(basename "$src")" > "$W/$tag.log" 2>&1 ) || {
        echo "  (构建失败，日志尾)"; tail -4 "$W/$tag.log" | sed 's/^/      /'; return 1; }
    echo "$d/build/$(basename "${src%.px}")"
}
# 跑一档；$1=bin $2=档（norm|stress）$3=期望输出正则
run_track() {
    local bin="$1" mode="$2" pat="$3" out rc
    if [ "$mode" = stress ]; then
        out=$(PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1 timeout -k 5 120 "$bin" 2>&1); rc=$?
    else
        out=$(timeout -k 5 120 "$bin" 2>&1); rc=$?
    fi
    echo "$out" > "$W/last_$mode.out"
    [ "$rc" = 0 ] || { echo "rc=$rc"; return 1; }
    echo "$out" | grep -qE "$pat" || { echo "输出不符"; return 1; }
    echo "$out" | grep -q "PX_GC_LIVECHK" && { echo "LIVECHK 响亮"; return 1; }
    return 0
}

step "① 静态：审计器自证 + --grow 下 vm.c 候选 0"
if python3 selfhost/gcroot_audit.py --self-test > "$W/selftest.log" 2>&1; then
    grep -q 'self-test: 10 通过 / 0 失败' "$W/selftest.log" && ok "自证 10/10（6 必中 + 4 必不中）" \
        || { bad "自证结论行不符"; tail -6 "$W/selftest.log" | sed 's/^/      /'; }
else
    bad "自证脚本失败"; tail -6 "$W/selftest.log" | sed 's/^/      /'
fi
python3 selfhost/gcroot_audit.py --grow --files runtime/vm.c > "$W/vmgrow.log" 2>&1
grep -q '候选 0' "$W/vmgrow.log" && ok "runtime/vm.c（--grow）候选 0 —— 缺陷 268 收口" \
    || { bad "runtime/vm.c 仍有候选"; sed 's/^/      /' "$W/vmgrow.log"; }
python3 selfhost/gcroot_audit.py --grow > "$W/growall.log" 2>&1
GROWN=$(grep -o '候选 [0-9]*' "$W/growall.log" | head -1 | awk '{print $2}')
echo "  ℹ️ 全仓 --grow 候选 = ${GROWN:-?}（缺陷 271 族 · 本轮只登记不修 · 见 docs/GC_ROOTS.md §8）"
sed -n '3,24p' "$W/growall.log" | sed 's/^/      /'
python3 selfhost/gcroot_audit.py > "$W/legacy.log" 2>&1
grep -o '候选 [0-9]*' "$W/legacy.log" | head -1 | sed 's/^/  ℹ️ 默认（legacy）全仓 /'

step "② 静态：S13 —— 「构造与登记之间不得有安全点」"
NPAIR=$(grep -c -A1 -E '^[[:space:]]*px_root_push\(\);' runtime/runtime.c runtime/vm.c 2>/dev/null | awk -F: '{s+=$2} END{print s+0}')
ADJ=$(python3 - <<'PY'
import re
n = 0
for p in ('runtime/runtime.c', 'runtime/vm.c', 'runtime/coro.c'):
    s = open(p, encoding='utf-8').read()
    n += len(re.findall(r'px_root_push\(\);[ \t]*\n[ \t]*PX_KEEP\(', s))
print(n)
PY
)
[ "$ADJ" = 0 ] && ok "相邻两段式 px_root_push()+PX_KEEP( = 0" || bad "仍有 $ADJ 处相邻两段式（应用 px_root_push_keep）"
grep -q 'void px_root_push_keep(LXValue v)' runtime/runtime.c && ok "原子原语 px_root_push_keep 实现在位" || bad "px_root_push_keep 未实现"
grep -q '登记时该对象已被回收' runtime/runtime.c && ok "px_root_push_keep 带 LIVECHK 自检（登记即死 ⇒ 响亮）" || bad "缺自检"
grep -q '若 (g_gc_livechk' runtime/runtime.c 2>/dev/null || true

step "③ 静态：S14 —— 回收出口不得清 g_tmp_root（缺陷 269 守卫）"
if grep -n '^[[:space:]]*g_tmp_root = NULL;' runtime/runtime.c | grep -v '1457' | grep -q .; then
    bad "px_gc_collect 内仍有 g_tmp_root = NULL;"
    grep -n '^[[:space:]]*g_tmp_root = NULL;' runtime/runtime.c | sed 's/^/      /'
else
    ok "两条回收路径都不清 g_tmp_root（仅声明处初值）"
fi

step "④ 动态：probe_lit / probe_coro 两档"
LIT=$(build_probe examples/m208_vm_c_local_roots/probe_lit.px lit) || bad "probe_lit 构建失败"
CORO=$(build_probe examples/m208_vm_c_local_roots/probe_coro.px coro) || bad "probe_coro 构建失败"
if [ -n "${LIT:-}" ]; then
    run_track "$LIT" norm '^SUM=[0-9]+$' && ok "probe_lit 正常档" || bad "probe_lit 正常档失败"
    run_track "$LIT" stress '^SUM=[0-9]+$' && ok "probe_lit 压力档" || { bad "probe_lit 压力档失败"; tail -4 "$W/last_stress.out" | sed 's/^/      /'; }
fi
if [ -n "${CORO:-}" ]; then
    run_track "$CORO" norm 'M208-PROBE-OK total=926480' && ok "probe_coro 正常档" || bad "probe_coro 正常档失败"
    run_track "$CORO" stress 'M208-PROBE-OK total=926480' && ok "probe_coro 压力档" || { bad "probe_coro 压力档失败"; tail -6 "$W/last_stress.out" | sed 's/^/      /'; }
fi

step "⑤ 动态回归：m93_s3/coro_gc_block 压力档（修前必红）"
CG=$(build_probe examples/m93_s3/coro_gc_block.px cg) || bad "coro_gc_block 构建失败"
if [ -n "${CG:-}" ]; then
    out=$(PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1 PX_GC_THRESHOLD=4000 PX_CORO_WORKERS=8 \
          timeout -k 5 150 "$CG" 2>&1); rc=$?
    if [ "$rc" = 0 ] && echo "$out" | grep -q "CORO-GC-BLOCK OK" && ! echo "$out" | grep -q "PX_GC_LIVECHK"; then
        ok "coro_gc_block 压力档（rc=0 · 零 LIVECHK）"
    else
        bad "coro_gc_block 压力档失败（rc=$rc）"; echo "$out" | tail -4 | sed 's/^/      /'
    fi
fi

# ---------------- 负控 ----------------
if [ "$NEG_SKIP" = 1 ]; then
    echo; echo "=== ⑥ 负控：--neg-skip（CI 跳过）==="; ok "负控跳过（CI 用）"
else
    step "⑥ 负控 A：恢复「回收出口清 g_tmp_root」⇒ ④ 压力档必红"
    restore_all
    patch_one runtime/runtime.c \
      '        // M208（缺陷 269）：**不得清 g_tmp_root** —— 见 px_gc_collect 出口注释。
        if (g_obj_count >= g_gc_threshold) g_gc_threshold = g_obj_count * 2;
        gc_debug("collect #%d(并发):' \
      '        g_tmp_root = NULL;   // NEGCTL-208A
        if (g_obj_count >= g_gc_threshold) g_gc_threshold = g_obj_count * 2;
        gc_debug("collect #%d(并发):' NEGCTL-208A
    A=$(build_probe examples/m208_vm_c_local_roots/probe_coro.px nA) || bad "负控 A 构建失败"
    if [ -n "${A:-}" ]; then
        if run_track "$A" stress 'M208-PROBE-OK' >/dev/null 2>&1; then
            bad "负控 A 未判红（缺陷 269 未能复现）"
        else
            grep -q "登记时该对象已被回收\|PX_GC_LIVECHK" "$W/last_stress.out" \
              && ok "负控 A 判红：$(grep -ao '登记时该对象已被回收\|PX_GC_LIVECHK] 读到' "$W/last_stress.out" | head -1)" \
              || { bad "负控 A 判红但理由不符"; tail -3 "$W/last_stress.out" | sed 's/^/      /'; }
        fi
    fi
    restore_all

    step "⑥ 负控 B：撤 PXOP_NEWDICT 的注册 + 令 g_tmp_root 离开 d ⇒ ④ 压力档必红"
    # 为什么要「+ pad」：缺陷 269 的修复（g_tmp_root 不清）会**吸收**单纯的「撤登记」
    #   （容器是新登记的 ⇒ g_tmp_root 一直指着它）⇒ 单纯撤登记已**不可判红**（M183 同款教训：
    #   新修复吸收旧缺陷 ⇒ 负控须换更强形态）。这里再插一次分配使 g_tmp_root 移开 ⇒
    #   该站点**自身**的登记成为必需，缺陷 268 在压力档复现。
    patch_one runtime/vm.c \
      '            px_root_push_keep(d);
            int n = (int)in.c;' \
      '            px_str("negctl-pad");   // NEGCTL-208B：撤登记 + 令 g_tmp_root 离开 d
            int n = (int)in.c;' NEGCTL-208B
    B=$(build_probe examples/m208_vm_c_local_roots/probe_coro.px nB) || bad "负控 B 构建失败"
    if [ -n "${B:-}" ]; then
        if run_track "$B" stress 'M208-PROBE-OK' >/dev/null 2>&1; then
            bad "负控 B 未判红（缺陷 268 NEWDICT 未能复现）"
        else
            grep -q "px_dict_set" "$W/last_stress.out" \
              && ok "负控 B 判红：$(grep -ao 'LIVECHK] 读到\*\*已回收对象\*\*（[^）]*）' "$W/last_stress.out" | head -1)" \
              || { bad "负控 B 判红但理由不符"; tail -3 "$W/last_stress.out" | sed 's/^/      /'; }
        fi
    fi
    restore_all

    step "⑥ 负控 C：撤 px_list_n 的注册 + 令 g_tmp_root 离开 v ⇒ ④ 压力档必红"
    patch_one runtime/runtime.c \
      '    px_root_push_keep(v);
    for (int i = 0; i < n; i++) px_list_push(v, items[i]);
    px_root_pop();' \
      '    px_str("negctl-pad");   // NEGCTL-208C：撤登记 + 令 g_tmp_root 离开 v
    for (int i = 0; i < n; i++) px_list_push(v, items[i]);' NEGCTL-208C
    C=$(build_probe examples/m208_vm_c_local_roots/probe_coro.px nC) || bad "负控 C 构建失败"
    if [ -n "${C:-}" ]; then
        if run_track "$C" stress 'M208-PROBE-OK' >/dev/null 2>&1; then
            bad "负控 C 未判红（px_list_n 漏根未能复现）"
        else
            grep -q "px_list_push" "$W/last_stress.out" \
              && ok "负控 C 判红：$(grep -ao 'LIVECHK] 读到\*\*已回收对象\*\*（[^）]*）' "$W/last_stress.out" | head -1)" \
              || { bad "负控 C 判红但理由不符"; tail -3 "$W/last_stress.out" | sed 's/^/      /'; }
        fi
    fi
    restore_all
fi

echo
echo "════════════ M208 verify：PASS=$PASS FAIL=$FAIL ════════════"
[ "$FAIL" = 0 ]
