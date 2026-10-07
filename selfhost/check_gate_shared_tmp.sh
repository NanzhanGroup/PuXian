#!/usr/bin/env bash
# ============================================================
# M264 · **门之间共用固定 /tmp 路径** —— 静态守卫
# ------------------------------------------------------------
# 病灶（「门间隔离」，M260 §八 候选 ③）：
#   两个门若共用**同一条固定 `/tmp` 路径**，那么 A 门留下的残留（文件/目录/进程）
#   会**改变 B 门的行为** ⇒ 「单独跑全绿、串起来跑却红」这类不可复现的怪现象
#   （M260 实测过一例：m157 跑完后 m260 探针 5/5 被杀，单独跑全绿）。
# 判据（白名单制 —— 共享**可以是**有意的，但必须**显式登记 + 写理由**）：
#   J1 被 **≥2 个门**共用的固定 `/tmp` 顶层名 **必须**在白名单里（否则判红并指名门）
#   ⚠️ 「固定名」的判据（M287s2）：匹配后紧跟 `$` / `{` / `*` 的**不算**
#      （`/tmp/devbuild_${name}.fp` 是**模板**、`/tmp/devbuild_*` 是**通配** —— 都不是固定路径）
#   J2 白名单每条必须有**非空理由**（否则 rc=3：无理由豁免 = 把红当绿）
#   J3 白名单**过期**（登记了但实测 ≤1 个门在用）⇒ 判红（防「把红当绿记下来」）
#   J4 规模锚点（扫描门数 · 共用名数有下限）—— 防判据静默变空
# 用法：
#   bash selfhost/check_gate_shared_tmp.sh [--root .] [--verbose]
#   bash selfhost/check_gate_shared_tmp.sh --self-test
# 退出码：0=通过 · 1=判红 · 2=用法错 · 3=判据自身失效
# ============================================================
set -uo pipefail
ROOT=.
VERBOSE=0
SELFTEST=0
while [ $# -gt 0 ]; do
  case "$1" in
    --root) shift; ROOT=${1:-.} ;;
    --verbose) VERBOSE=1 ;;
    --self-test) SELFTEST=1 ;;
    *) echo "未知参数 $1"; exit 2 ;;
  esac
  shift
done
ROOT=$(cd "$ROOT" && pwd)
WL="$ROOT/selfhost/gate_shared_tmp.txt"

scan() {   # $1=仓库根 → 打印 "名字<TAB>门1,门2,..."
  python3 - "$1" <<'PY'
import io, os, re, sys, collections
root = sys.argv[1]
RX = re.compile(r'/tmp/([A-Za-z0-9_.-]+)')


def strip_hash_comments(text):
    """去掉行内注释（quote 感知）—— **注释里的路径不算「引用」**。
    ⚠️ 自伤（本轮踩）：M264 给 m260 写的说明注释里出现了 `/tmp/px_m256_probe`，
       不剥注释会把「已经修好的门」当成「仍在共用」⇒ 假红。"""
    # ⚠️ 已知边界：本剥离器**逐行**判定引号状态 ⇒ 跨行引号串里的路径仍会被计入
    #   （M264 实测：m260 的一条两行 echo 里复述了旧路径 ⇒ 把「已修好的门」读成「仍共用」）。
    #   处置：**不要在门里复述旧路径的字面量**（判据与文档一致性由人工守）。
    out = []
    for line in text.split('\n'):
        q = None
        i = 0
        while i < len(line):
            c = line[i]
            if q:
                if c == '\\':
                    i += 2
                    continue
                if c == q:
                    q = None
            elif c in '\'"':
                q = c
            elif c == '#':
                break
            i += 1
        out.append(line[:i])
    return '\n'.join(out)
own = collections.defaultdict(set)
ex = os.path.join(root, 'examples')
for d in sorted(os.listdir(ex)):
    p = os.path.join(ex, d)
    if not os.path.isdir(p):
        continue
    for fn in sorted(os.listdir(p)):
        if not (fn.endswith('.px') or fn.endswith('.sh')):
            continue
        try:
            s = io.open(os.path.join(p, fn), encoding='utf-8', errors='replace').read()
        except Exception:
            continue
        t = strip_hash_comments(s)
        for m in RX.finditer(t):
            # ⚠️ **模板/通配 ≠ 固定名**（M287s2 真因修正）：`/tmp/devbuild_${name}.fp` 与
            #   `/tmp/devbuild_*` 的匹配结果是 `devbuild_`，但它**不是**任何门真正读写的
            #   固定路径，而是「命名空间」。把它当固定名 ⇒ 两个只是**提到**它的门被判「共用」
            #   ⇒ 假红（M287s2 实测：m221 的 grep 模式 + m283 的说明）。
            #   判据：匹配后紧跟 `$`/`{`/`*` ⇒ 模板/通配，跳过（**固定名照旧统计**）。
            if m.end() < len(t) and t[m.end()] in '$*{':
                continue
            own[m.group(1)].add(d)
for name, gates in sorted(own.items()):
    print("%s\t%s" % (name, ','.join(sorted(gates))))
PY
}

if [ "$SELFTEST" = 1 ]; then
  W=$(mktemp -d)
  mkdir -p "$W/examples/a" "$W/examples/b" "$W/examples/c" "$W/selfhost"
  printf '/tmp/shared1\n' > "$W/examples/a/verify.sh"
  printf '/tmp/shared1\n' > "$W/examples/b/verify.sh"
  printf '/tmp/solo1\n'   > "$W/examples/c/verify.sh"
  # ⚠️ 自伤：规模锚点（门数 ≥150）在 3 个门的夹具上**必然**触发 ⇒ 自证会假红。
  #   夹具必须满足锚点（否则测的是锚点、不是判据）。
  for _i in $(seq 1 150); do mkdir -p "$W/examples/g$_i"; done
  n=0; tot=6
  # ① 未登记共用 ⇒ 判红并指名
  : > "$W/selfhost/gate_shared_tmp.txt"
  OUT=$(bash "$0" --root "$W" 2>&1); RC=$?
  if [ "$RC" = 1 ] && echo "$OUT" | grep -q 'shared1'; then n=$((n+1)); echo "  ① 未登记共用 ⇒ rc=1 且指名 shared1  OK"
  else echo "  ① BAD（rc=$RC）：$OUT"; fi
  # ② 登记 + 理由 ⇒ 通过；白名单里的 solo1（只有 1 门用）⇒ 过期判红
  printf 'shared1\ta 与 b 有意共用（自证夹具）\n' > "$W/selfhost/gate_shared_tmp.txt"
  OUT=$(bash "$0" --root "$W" 2>&1); RC=$?
  [ "$RC" = 0 ] && { n=$((n+1)); echo "  ② 登记后 ⇒ rc=0  OK"; } || echo "  ② BAD（rc=$RC）：$OUT"
  printf 'shared1\t理由甲\nsolo1\tsolo1 只有一门用（应判过期）\n' > "$W/selfhost/gate_shared_tmp.txt"
  OUT=$(bash "$0" --root "$W" 2>&1); RC=$?
  if [ "$RC" = 1 ] && echo "$OUT" | grep -q '过期'; then n=$((n+1)); echo "  ③ 白名单过期 ⇒ rc=1  OK"
  else echo "  ③ BAD（rc=$RC）：$OUT"; fi
  # ④ 无理由 ⇒ rc=3
  printf 'shared1\t\n' > "$W/selfhost/gate_shared_tmp.txt"
  OUT=$(bash "$0" --root "$W" 2>&1); RC=$?
  if [ "$RC" = 3 ] && echo "$OUT" | grep -q '无理由\|理由'; then n=$((n+1)); echo "  ④ 无理由 ⇒ rc=3  OK"
  else echo "  ④ BAD（rc=$RC）：$OUT"; fi
  # ⑤/⑥ 模板/通配**不算**「固定名」，但同一份夹具里的**固定**形式必须仍被统计
  #   （M287s2：m221 的 grep 模式 + m283 的说明里有 `/tmp/devbuild_${name}.fp` ⇒ 曾被判共用）
  printf '/tmp/tpl_${name}.x\n/tmp/tpl_fixed\n' > "$W/examples/a/verify.sh"
  printf '/tmp/tpl_${name}.x\n/tmp/tpl_fixed\n' > "$W/examples/b/verify.sh"
  printf 'tpl_fixed\ttpl_fixed 两门共用（自证夹具）\n' > "$W/selfhost/gate_shared_tmp.txt"
  OUT=$(bash "$0" --root "$W" --verbose 2>&1); RC=$?
  if [ "$RC" = 0 ]; then n=$((n+1)); echo "  ⑤ 模板/通配形式不计 ⇒ rc=0  OK"
  else echo "  ⑤ BAD（rc=$RC）：$OUT"; fi
  if echo "$OUT" | grep -q 'tpl_fixed' && ! echo "$OUT" | grep -q 'tpl_ '; then
      n=$((n+1)); echo "  ⑥ 反向判据：固定名**仍计** / 模板名**不计**  OK"
  else echo "  ⑥ BAD（判据被改瞎）：$OUT"; fi
  rm -rf "$W"
  echo "self-test: $n/$tot"
  [ "$n" = "$tot" ] && { echo "GATE-SHARED-TMP-SELFTEST-OK"; exit 0; }
  echo "GATE-SHARED-TMP-SELFTEST-FAIL"; exit 1
fi

MAP=$(scan "$ROOT") || { echo "❌ 扫描失败"; exit 3; }
NGATE=$(ls -d "$ROOT"/examples/*/ 2>/dev/null | wc -l)
NSHARED=$(printf '%s\n' "$MAP" | awk -F'\t' 'NF==2 && index($2,",")>0' | wc -l)
[ "$VERBOSE" = 1 ] && printf '%s\n' "$MAP" | awk -F'\t' 'index($2,",")>0 {printf "   · %-24s %s\n", $1, $2}'

declare -A REASON
if [ -f "$WL" ]; then
  while IFS= read -r line; do
    case "$line" in ''|'#'*) continue ;; esac
    nm=${line%%$'\t'*}; rs=${line#*$'\t'}
    if [ "$nm" = "$line" ] || [ -z "${rs// /}" ]; then
      echo "❌ 白名单 `$nm` **无理由**（判据失效：无理由豁免 = 把红当绿）"; exit 3
    fi
    REASON[$nm]="$rs"
  done < "$WL"
fi

MISSING=0
while IFS=$'\t' read -r name gates; do
  [ -z "${name:-}" ] && continue
  case "$gates" in *,*) ;; *) continue ;; esac
  if [ -z "${REASON[$name]:-}" ]; then
    MISSING=$((MISSING+1))
    echo "  · ❌ 未登记：$name ⇐ $gates"
  fi
done <<< "$MAP"

STALE=0
for nm in "${!REASON[@]}"; do
  g=$(printf '%s\n' "$MAP" | awk -F'\t' -v n="$nm" '$1==n {print $2}')
  case "$g" in
    ''|*,*) ;;                              # 无此名（实测已消失）或仍 ≥2 门 ⇒ 不算过期
    *) STALE=$((STALE+1)); echo "  · ❌ 白名单过期：$nm（实测只有 $g 在用）" ;;
  esac
done

FAIL=0
[ "$NGATE" -lt 150 ] && { echo "❌ 规模锚点失效：扫描到 $NGATE 个门（期望 ≥150）"; FAIL=1; }
[ "$NSHARED" -lt 1 ] && { echo "❌ 规模锚点失效：共用名 $NSHARED 个（期望 ≥1）"; FAIL=1; }

if [ "$MISSING" != 0 ] || [ "$STALE" != 0 ] || [ "$FAIL" = 1 ]; then
  echo "❌ 门间共用固定 /tmp 路径：未登记 $MISSING 个 · 白名单过期 $STALE 个"
  echo "   ⇒ 修法：① 让该门用自己的目录（推荐：环境变量参数化，如 M264 对 M256_PROBE_D 的做法）；"
  echo "      ② 确实有意共享的，在 selfhost/gate_shared_tmp.txt 里**写清理由**。"
  exit 1
fi
echo "✅ 门间隔离：扫描 $NGATE 个门 · 共用固定 /tmp 名 $NSHARED 个**全部已登记**（各带理由）"
exit 0
