#!/bin/bash
# M239 门 · **match / case 模式族** 三轨全量对拍（缺陷 382–392）
#
# 主题：M199（native 函数面）· M226（方法面）· M227（同名两门）· M228（三个门）·
#   M229（tuple/result）· M230（索引/切片）· M231（运算符矩阵）· M232（真值性/短路）·
#   M233（兜底渲染器）· M234（bytes 族）· M237（字段/构造/变体）逐个推过去 ——
#   **`match` / `case` 这一整个控制流构造从来没有被清单级度量过**。
#   全仓只有 13 个 match 站点、一个 234 字节的裸示例；而它一条语义有三个实现
#   （解释轨 iexpr · VM 轨 bc_emit · C 轨 cg_expr），三者互不相同。
#
# 修前基线（80 例三轨实测）：**34 例分叉**，其中「静默错值」5 类：
#   382 interp/C **完全不求值 guard**（`case 5 if false:` 照样取该 arm —— 连副作用都没有）
#   384 编译两轨**不绑定**模式变量（`case n:` 的 n 未定义 / 读到外层同名）
#   387 编译两轨 tuple 模式恒不匹配（C 只看 items[0]）
#   388 interp 的 `return` 落在 arm 体内被表达式 Block **吞成值**
#   389 interp 的 break/continue 在 arm 体内报 R1005
#   391 全不匹配时 C/VM **静默返回 subject 值**（`let r = match 9: case 1: "one"` ⇒ r == 9）
#   392 构造器模式不查 arity（`case Red(1)` 对无载荷变体照匹配）
#
# 定稿（三轨一条真相，参考 = 解释轨单一 env）：
#   ① guard 只在模式命中后求值，按 M232 真值性判定，最多一次；
#   ② `case x:` / `case (a,b):` 绑定**作用域 = 该 arm**（guard + body），**遮蔽**外层同名；
#   ③ tuple 模式：subject ∈ {tuple, list} 且长度**恰等**，逐元素递归；
#   ④ 无载荷变体带子模式 ⇒ 永不匹配（data enum 尚未支持，登记为覆盖边界）；
#   ⑤ 全不匹配（含 guard 全假）⇒ **R1003 响亮**（`match 未匹配任何分支（非穷尽）：<str(subject)>`）；
#   ⑥ arm 体内的 return/break/continue **正常传播**。
#
# 判据层：[1] 静态 · [2] 三轨对拍（80 例 × 3 轨）· [3] MODEL.tsv 双向 ·
#        [4][5][6] 负控 A/B/C（各自独立判红 + 源逐字节还原）· [7] 负控 D（判据自伤）· [8] 覆盖边界
# CI 用 `--neg-skip`（负控各要重建一次编译器）。
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D="$ROOT/examples/m239_match_case"
W=/tmp/m239_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
has() { grep -qF "$2" "$ROOT/$1"; }
cnt() { grep -cF "$2" "$ROOT/$1" || true; }
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }
# M239s1（缺陷 394）：① 进门记 selfhost/ 指纹 ⇒ 门尾断言「门内未改动」（负控残留自检）；
#   ② trap 兜底还原（中途被杀/异常退出也不会把负控补丁留在源码里）。
SELFHOST_SHA_IN=$(cd "$ROOT" && sha256sum selfhost/*.px 2>/dev/null | sha256sum | cut -d' ' -f1)
trap 'NEGCTL --restore >/dev/null 2>&1; true' EXIT
# 正判据用**入库件**（CI 里也在）；负控用 devbuild 重编的 dev 件
TRACKS() { python3 "$D/three_tracks.py" --root "$ROOT" --work "$W/t$1"; }
TRACKS_DEV() { python3 "$D/three_tracks.py" --root "$ROOT" --work "$W/t$1" --dev; }

echo "── [1] 静态判据（新形态在位 + 旧形态清零 + 规模锚点）"
chk "runtime：px_match_fail 在位（三轨非穷尽唯一出口）" "has runtime/runtime.c 'LXValue px_match_fail(LXValue subj)'"
chk "runtime：px_match_tuple 在位（元组模式结构判据）" "has runtime/runtime.c 'int px_match_tuple(LXValue v, int n)'"
chk "vm.h：PXOP_MATCHFAIL 在位" "has runtime/vm.h 'PXOP_MATCHFAIL 71'"
chk "vm.h：PXOP_MATCHTUP 在位" "has runtime/vm.h 'PXOP_MATCHTUP 72'"
chk "VM 轨：绑定走新临时槽 + smap 快照遮蔽" "has selfhost/bc_emit.px 'def bc_bind_var(func, name, t)'"
chk "VM 轨：guard 求值在位" "has selfhost/bc_emit.px 'if arm[2] != null:'"
chk "C 轨：cg_match_bind（_mbN 专用计数器，不占 _vN）" "has selfhost/cg_expr.px 'def cg_match_bind(name):'"
chk "C 轨：非穷尽走 px_match_fail" "has selfhost/cg_expr.px 'px_match_fail('"
chk "解释轨：guard 求值在位" "has selfhost/iexpr.px 'guard_ok = i_truthy(gr.unwrap())'"
chk "解释轨：arm body 走语句路径（控制流可传播）" "has selfhost/iexpr.px 'def i_eval_arm_body(b, env):'"
chk "解释轨：Match 语句上传播控制流" "has selfhost/istmt.px 'stmt[1][0] == \"Match\" and type(ev) == \"dict\"'"
chk "旧形态清零：C 轨 PatBinding 不再恒 true" "[ \"\$(cnt selfhost/cg_expr.px 'return \\\"true\\\"')\" -le 1 ]"
chk "旧形态清零：VM 轨 bc_match_cond 不再返回槽（改 jumps 列表）" "[ \"\$(grep -c 'def bc_match_cond(pattern, t, func, jumps)' selfhost/bc_emit.px)\" -eq 1 ]"
chk "规模锚点：用例 ≥ 80" "[ \$(wc -l < $D/cases.txt) -ge 80 ]"
chk "规模锚点：驱动器 ≥ 3 个模式类（binding/tuple/guard）" "[ \$(grep -cE 'case n:|case \(a, b\):|case .* if ' $D/drv.px) -ge 4 ]"
# ⚠️ M244（缺陷 413）：判据要**与判据前比**，不能与「空」比。
#   原实现 `[ -z "$(git status --short selfhost/ | grep -v '^ M')" ]` 的真实语义是
#   「**除了已跟踪文件的未暂存修改之外，工作树必须是干净的**」——
#   于是 `selfhost/` 下**任何未跟踪的新文件**（例如本轮新增的 `gate_par.py`）
#   都会让这条判红，而它与「负控还原」**毫无关系** ⇒ **假红**（实测：本门 rc=1，
#   而全部 22 条其它判据都 PASS）。
#   正确判据 = **快照前后比对**（这也正是那句提示文字本来要说的意思）。
SS0="$(git -C $ROOT status --short selfhost/)"
for c in A B C D; do
  chk "负控锚点 $c 可应用（自证：不改源，仅校验命中唯一）" "NEGCTL --apply $c >/dev/null 2>&1 && NEGCTL --restore >/dev/null 2>&1"
done
SS1="$(git -C $ROOT status --short selfhost/)"
chk "负控后源码逐字节还原（与**判据前**一致，不是与空比）" "[ \"$SS1\" = \"$SS0\" ]"

echo "── [2] 三轨对拍（80 例 × 3 轨 = 240 次执行）"
if TRACKS 1 >"$W/t1.log" 2>&1; then
  chk "三轨一致（$(grep -o 'OK=[0-9]*' "$W/t1.log" | head -1)）" "grep -q 'FORK=0' '$W/t1.log'"
else
  chk "三轨一致（分叉不为 0）" false; head -20 "$W/t1.log"
fi

echo "── [3] MODEL.tsv 双向核对"
python3 - "$ROOT" "$W" <<'PY' >"$W/model.log" 2>&1
import os, re, subprocess, sys
ROOT, W = sys.argv[1], sys.argv[2]
D = os.path.join(ROOT, 'examples/m239_match_case')
def norm(t):
    o = []
    for L in t.split('\n'):
        L = L.rstrip()
        if not L: continue
        L = re.sub(r'^运行时错误\s*\[[^\]]*\]:\s*', '', L)
        L = re.sub(r'^运行时错误:\s*', '', L)
        L = re.sub(r'^错误\s*\[R([0-9]+)\]\s*[0-9]+:[0-9]+:\s*', r'R\1: ', L)
        L = re.sub(r'^R([0-9]+):\s*', r'R\1: ', L)
        o.append(L)
    return o
model = {}
for L in open(D + '/MODEL.tsv'):
    if L.startswith('#') or L.startswith('例号'): continue
    p = L.rstrip('\n').split('\t')
    if len(p) >= 5: model[p[0]] = (p[3], p[4])
cases = [L.strip() for L in open(D + '/cases.txt') if L.strip()]
bad = 0; missing = 0
for c in cases:
    e = dict(os.environ); e['M239C'] = c
    PXI = os.environ.get('M239_PXI', os.path.join(ROOT, 'bootstrap/pxi'))
    r = subprocess.run([PXI, os.path.join(D, 'drv.px')],
                       capture_output=True, text=True, env=e, timeout=30)
    lines = norm(r.stdout + r.stderr)
    err = (r.returncode != 0) or bool(lines and re.match(r'^R[0-9]{4}: ', lines[0]))
    kind = 'ERR' if err else 'VAL'
    exp = (lines[0] if lines else 'R????') if err else ';'.join(lines)
    if c not in model:
        print('MISS %s 未登记' % c); missing += 1; continue
    if model[c] != (kind, exp):
        print('MISMATCH %s 表=%s 实测=%s' % (c, model[c], (kind, exp))); bad += 1
for k in model:
    if k not in cases:
        print('STALE %s 表里有、实测没有' % k); bad += 1
print('MODEL OK=%d MISS=%d BAD=%d STALE=%d' % (len(cases) - missing - bad, missing, bad,
      sum(1 for k in model if k not in cases)))
sys.exit(1 if (missing or bad) else 0)
PY
chk "MODEL.tsv 双向（漏登记 / 过期 / 不符 都判红）" "grep -q 'MISS=0' '$W/model.log' && grep -q 'BAD=0' '$W/model.log' && grep -q 'STALE=0' '$W/model.log'"
grep -E 'MISMATCH|MISS |STALE' "$W/model.log" | head -5 || true

if [ "$NEG" = 1 ]; then
  echo "── [4] 负控 A：忠实撤回 VM 轨（绑定 / MATCHFAIL / MATCHTUP）"
  if NEGCTL --apply A >"$W/ncA.patch" 2>&1 && bash "$ROOT/selfhost/devbuild.sh" pxc --vm >"$W/ncA.dev" 2>&1; then
    if TRACKS_DEV A >"$W/ncA.log" 2>&1; then chk "负控 A 必红" false
    else chk "负控 A 必红（三轨分叉 $(grep -o 'FORK=[0-9]*' "$W/ncA.log" | head -1)）" "grep -q 'FORK=' '$W/ncA.log'"; fi
  else chk "负控 A 必红" false; tail -6 "$W/ncA.patch" "$W/ncA.dev"; fi
  NEGCTL --restore >/dev/null 2>&1

  echo "── [5] 负控 B：忠实撤回 C 轨（绑定 / guard / 非穷尽）"
  if NEGCTL --apply B >"$W/ncB.patch" 2>&1 && bash "$ROOT/selfhost/devbuild.sh" pxc >"$W/ncB.dev" 2>&1; then
    if TRACKS_DEV B >"$W/ncB.log" 2>&1; then chk "负控 B 必红" false
    else chk "负控 B 必红（三轨分叉 $(grep -o 'FORK=[0-9]*' "$W/ncB.log" | head -1)）" "grep -q 'FORK=' '$W/ncB.log'"; fi
  else chk "负控 B 必红" false; tail -6 "$W/ncB.patch" "$W/ncB.dev"; fi
  NEGCTL --restore >/dev/null 2>&1

  echo "── [6] 负控 C：忠实撤回解释轨（guard / 控制流传播）"
  if NEGCTL --apply C >"$W/ncC.patch" 2>&1 && bash "$ROOT/selfhost/devbuild.sh" pxi >"$W/ncC.dev" 2>&1; then
    if TRACKS_DEV C >"$W/ncC.log" 2>&1; then chk "负控 C 必红" false
    else chk "负控 C 必红（三轨分叉 $(grep -o 'FORK=[0-9]*' "$W/ncC.log" | head -1)）" "grep -q 'FORK=' '$W/ncC.log'"; fi
  else chk "负控 C 必红" false; tail -6 "$W/ncC.patch" "$W/ncC.dev"; fi
  NEGCTL --restore >/dev/null 2>&1

  echo "── [7] 负控 D：判据自伤（三轨比对改恒等 ⇒ A 的红必须消失）"
  if NEGCTL --apply A >/dev/null 2>&1 && NEGCTL --apply D >/dev/null 2>&1 \
     && bash "$ROOT/selfhost/devbuild.sh" pxc --vm >"$W/ncD.dev" 2>&1; then
    chk "负控 D：A 的红被消除（证明该红来自比对判据本身）" "TRACKS_DEV D >'$W/ncD.log' 2>&1"
  else chk "负控 D" false; tail -4 "$W/ncD.dev"; fi
  NEGCTL --restore >/dev/null 2>&1
  bash "$ROOT/selfhost/devbuild.sh" pxc pxi --vm >"$W/rebuild.log" 2>&1
  chk "负控后 dev 件重建回基线" "[ -x /tmp/pxcdev ] && [ -x /tmp/pxidev ] && [ -x /tmp/pxcdev_vm ]"
else
  echo "── [4]-[7] 负控（--neg-skip：跳过，需重建编译器）"
fi

echo "── [8] 负控残留自检（M239s1 缺陷 394）"
SELFHOST_SHA_OUT=$(cd "$ROOT" && sha256sum selfhost/*.px 2>/dev/null | sha256sum | cut -d' ' -f1)
chk "门内 selfhost/*.px 指纹不变（快照=进门态 ⇒ 负控还原到底）" "[ '$SELFHOST_SHA_IN' = '$SELFHOST_SHA_OUT' ]"

echo "── [9] 覆盖边界（如实登记）"
cat <<'EOF'
  · parse 限制：`case -1:`（负字面量模式）报 E2001「无效的模式: -」；`case 1 + 2:` 报「期望 ':'」；
    多行 match 不能直接作函数实参 / 括号内表达式（`print(str(match x: …))` ⇒ 「期望 缩进块」）
    ⇒ 本门全部用例都写成「先 let 绑定、再使用」的形态。
  · 构造器模式带子模式（`case Red(1)`）对**无载荷**变体**永不匹配**（data enum 尚未支持，
    见缺陷 364）⇒ 只验「三轨同判不匹配」，不验 payload 绑定。
  · 绑定被 arm 内闭包捕获（`case n: let f = fn() { n }`）不在面内（VM 的 cell 装箱未覆盖该路径）。
  · `match` 的**优先级**（与 `+`/三元/管道的组合）不在面内（M165 已管求值顺序）。
  · 解释轨 `iexpr` 的**表达式 Block**（非 match 位置）仍保留「return 拆成值」的旧行为 ⇒ 本门未覆盖。
EOF

echo
echo "M239-VERIFY-$([ $fail -eq 0 ] && echo OK || echo FAIL) pass=$pass fail=$fail"
[ $fail -eq 0 ] || exit 1
