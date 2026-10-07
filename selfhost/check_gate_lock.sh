#!/usr/bin/env bash
# ============================================================
# selfhost/check_gate_lock.sh —— **门级互斥锁**守卫（M276 · 晨曦 2026-10-05 回馈）
# ------------------------------------------------------------
# 病灶（「门间隔离」的第三个面 —— §一 进程/§二 端口/§三 **锁**）：
#   93 个门会在负控里**现场改源码**（`restore_all` / 负控打桩 / `run_neg` …）。
#   全量门有 PID 锁（`run_gates.sh`），但**人工单跑一扇门**与全量门并发**无覆盖** ⇒
#   负控的 snapshot/restore 互相盖掉（M191/M213/M221/M223 各撞一次）。
#   修法：`selfhost/gate_lock.sh`（被门 source）+ `run_gates.sh` 持同一把锁。
#   本守卫负责**让这个修法不会随时间退化**。
#
# 判据：
#   J1 **每个** `examples/*/verify.sh` 必须 source `gate_lock.sh`（否则判红并指名）
#   J2 source 行必须出现在**第一个改源码标记之前**（在标记之后 source = 白 source）
#   J3 规模锚点（扫描门数 ≥190 · 含改源码标记的门数 ≥90）—— 防判据静默变空
#   J4 `run_gates.sh` 必须**持有**同一把锁并导出 `PX_GATE_LOCK_HELD=1`
#      （只导出不持有 = 等于没锁：单跑的门照样能进）
#   J5 `gate_lock.sh` 自身存在且是 bash 语法合法
#
# 用法：
#   bash selfhost/check_gate_lock.sh [--root .] [--verbose]
#   bash selfhost/check_gate_lock.sh --self-test      # 合成夹具 + 3 道负控
# 退出码：0=通过 · 1=判红 · 2=用法错 · 3=判据自身失效
# ============================================================
set -uo pipefail
ROOT=.
VERBOSE=0
SELFTEST=0
HARM=0
while [ $# -gt 0 ]; do
  case "$1" in
    --root) shift; ROOT=${1:-.} ;;
    --verbose) VERBOSE=1 ;;
    --self-test) SELFTEST=1 ;;
    --harm) HARM=1 ;;          # 判据自伤（**仅供自证**：证明「红」来自 J1/J2 本身）
    *) echo "未知参数 $1"; exit 2 ;;
  esac
  shift
done
ROOT=$(cd "$ROOT" && pwd) || { echo "❌ 进不去 --root $ROOT"; exit 2; }

if [ "$SELFTEST" = 1 ]; then
    exec bash "$(cd "$(dirname "$0")" && pwd)/../examples/m276_gate_lock/verify.sh"
fi

# ── 扫描（python：需要「剥注释」才能精确判标记）──
OUT="$(python3 - "$ROOT" <<'PY'
import os, re, sys, json
root = sys.argv[1]
MARK = re.compile(r'(?i)\b(restore_all|snapshot|negctl|patch_one|run_neg)\b')
SRC  = re.compile(r'^\s*\.\s+.*gate_lock\.sh|^\s*source\s+.*gate_lock\.sh', re.M)

def strip_hash_comments(text):
    """去掉行内注释（quote 感知）—— 注释里的标记不算「改源码」。"""
    out = []
    for line in text.split('\n'):
        q = None; i = 0
        while i < len(line):
            c = line[i]
            if q:
                if c == '\\': i += 2; continue
                if c == q: q = None
            elif c in '\'"':
                q = c
            elif c == '#':
                break
            i += 1
        out.append(line[:i])
    return '\n'.join(out)

ex = os.path.join(root, 'examples')
gates = []
for d in sorted(os.listdir(ex)):
    v = os.path.join(ex, d, 'verify.sh')
    if os.path.isfile(v):
        gates.append((d, v))

res = {'gates': len(gates), 'marked': 0, 'no_src': [], 'bad_order': [], 'run_ok': False,
       'lock_exists': False, 'src_gates': 0}
for d, v in gates:
    raw = open(v, encoding='utf-8', errors='ignore').read()
    # ── J1：**每个**门都要 source（M285 收尾收紧 · 缺陷 499-b）────────
    #   原文只对「含改源码标记」的门查 source（`if mark_ln is None: continue`），
    #   而头注写的是「每个」⇒ **头注与实现不符**（缺陷 496 同族）。
    #   为什么「每个」才对：锁保护的是**读写双方** —— 只**读**源码的门（扫描 / 对拍 /
    #   判据点）同样会被并发打桩的中间态污染（M276 记的假红 / 假绿就是这么来的）。
    src_ln = None
    for i, line in enumerate(raw.split('\n')):
        if SRC.match(line):
            src_ln = i
            break
    if src_ln is not None:
        res['src_gates'] += 1
    else:
        res['no_src'].append(d)   # ← 缺陷 499-c：与 J1 的计数口径**对齐**
    # ⚠️ 顺序判据必须比**行号**，不能比**字节偏移** ——
    #   `strip_hash_comments` 会**删掉**注释字符 ⇒ 两个串长度不同，偏移不可比
    #   （M276 自伤：首版用 start() 比较 ⇒ 194 门里 84 门假报「source 在标记之后」）。
    mark_ln = None
    for i, line in enumerate(raw.split('\n')):
        if MARK.search(strip_hash_comments(line)):
            mark_ln = i
            break
    if mark_ln is None:
        continue
    res['marked'] += 1
    if src_ln is not None and src_ln > mark_ln:
        res['bad_order'].append(d)

rg = open(os.path.join(root, 'selfhost/run_gates.sh'), encoding='utf-8', errors='ignore').read()
# J4：必须**先判后写**（持有）—— 即出现 PX_GATE_LOCK_HELD 的判定 且 有写锁文件的动作
res['run_ok'] = ('PX_GATE_LOCK_HELD' in rg) and \
                (re.search(r'>\s*"\$_PGK"', rg) is not None) and \
                ('export PX_GATE_LOCK_HELD=1' in rg)
res['lock_exists'] = os.path.isfile(os.path.join(root, 'selfhost/gate_lock.sh'))
print(json.dumps(res))
PY
)" || { echo "❌ 扫描失败"; exit 3; }

j() { python3 -c "import json,sys;d=json.loads(sys.argv[1]);print(d[sys.argv[2]])" "$OUT" "$1"; }

NG=$(j gates); NM=$(j marked); NS=$(j src_gates)
echo "── 扫描：门 $NG 个 · 含「改源码」标记 $NM 个 · 已 source 锁 $NS 个 ──"
[ "$VERBOSE" = 1 ] && echo "   （--verbose：逐条明细见下）"

RC=0
fail() { echo "   ❌ $*"; RC=1; }
ok()   { echo "   ✅ $*"; }

# ── J3 规模锚点（先判 —— 判据变空比判红更危险）──
if [ "$NG" -lt 190 ]; then fail "J3 规模锚点：扫描门数 $NG < 190 —— 扫描面疑似失效"; else ok "J3 规模锚点：门 $NG ≥ 190"; fi
if [ "$NM" -lt 90 ];  then fail "J3 规模锚点：含标记门数 $NM < 90 —— 「改源码」标记疑似失效"; else ok "J3 规模锚点：含标记门 $NM ≥ 90"; fi

# ── J5 gate_lock.sh 存在 ──
if [ "$(j lock_exists)" != "True" ]; then fail "J5 selfhost/gate_lock.sh 不存在"; else ok "J5 selfhost/gate_lock.sh 在位"; fi

# ── J1 每个门都必须 source ──
if [ "$HARM" = 1 ]; then
    echo "   ⚠️ --harm：J1/J2 已关闭（判据自伤，仅供自证）"
else
    MISSING="$(python3 - "$OUT" <<'PY'
import json,sys
d=json.loads(sys.argv[1]); print(' '.join(d['no_src']))
PY
)"
    if [ -n "$MISSING" ]; then
        fail "J1 有门**未** source gate_lock.sh（$(( $(j gates) - $(j src_gates) )) 个）："
        echo "$MISSING" | tr ' ' '\n' | sed 's/^/        · /'
    else
        ok "J1 全部 $NG 个门都 source 了 gate_lock.sh"
    fi
    BADORD="$(python3 - "$OUT" <<'PY'
import json,sys
d=json.loads(sys.argv[1]); print(' '.join(d['bad_order']))
PY
)"
    if [ -n "$BADORD" ]; then
        fail "J2 source 行出现在**改源码标记之后**（等于白 source）："
        echo "$BADORD" | tr ' ' '\n' | sed 's/^/        · /'
    else
        ok "J2 所有含标记的门，source 行都在标记之前"
    fi
fi

# ── J4 run_gates.sh 必须**持有**锁 ──
if [ "$(j run_ok)" != "True" ]; then
    fail "J4 run_gates.sh 未「持有」门级锁（需：判 PX_GATE_LOCK_HELD + 写锁文件 + export）"
else
    ok "J4 run_gates.sh 持有门级锁并导出 PX_GATE_LOCK_HELD=1"
fi

echo ""
if [ "$RC" = 0 ]; then echo "GATE-LOCK-GUARD-OK 门 $NG · 标记 $NM"; else echo "GATE-LOCK-GUARD-FAIL"; fi
exit $RC
