#!/usr/bin/env bash
# ============================================================
# selfhost/check_gate_registry.sh —— **门注册一致性守卫**（M243 建立）
# ------------------------------------------------------------
# 判据：`selfhost/gates.registry.sh`（本地权威清单）里的 example 门
#       ⇄ `.github/workflows/ci.yml` 里显式列出的 example 门
#       **双向一致**；差异只允许出现在**带理由的例外表**里。
#
# 为什么要有它（本仓反复出事故的形状）：
#   · M235 缺陷 355「native 名册同步了、门忘了注册」⇒ 用户可见面回归
#   · M237 缺陷 373 同族
#   · M243 建立时实测：本地 121 个 example 门 ⇄ CI 124 个 ⇒ **5 处不一致**
#     （1 个只在本地、4 个只在 CI）—— 此前全靠人工同步，早晚出事。
#
# 例外表纪律（沿用本仓口诀）：
#   ① 每条必须有**理由**；② 必须**仍然成立**（过期即判红）；
#   ③ 有**规模锚点**（防「提取器坏了 ⇒ 空集 ⊇ 空集」的假绿）。
# ============================================================
set -uo pipefail
cd "$(cd "$(dirname "$0")/.." && pwd)"

REG="${GATE_REGISTRY:-selfhost/gates.registry.sh}"
CI="${GATE_CI_YML:-.github/workflows/ci.yml}"
PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

T=$(mktemp -d); trap 'rm -rf "$T"' EXIT

echo "=== 门注册一致性（清单 ⇄ ci.yml · M243）==="

# ── 1 双向提取（都从**源码**派生，不手抄任何名单）──
grep -oE '^[[:space:]]*run[[:space:]]+[^[:space:]]+.*examples/[A-Za-z0-9_]+/verify\.sh' "$REG" \
  | grep -oE 'examples/[A-Za-z0-9_]+/verify\.sh' | sort -u > "$T/local.txt"
grep -oE 'bash[[:space:]]+examples/[A-Za-z0-9_]+/verify\.sh' "$CI" \
  | grep -oE 'examples/[A-Za-z0-9_]+/verify\.sh' | sort -u > "$T/ci.txt"

NL=$(wc -l < "$T/local.txt"); NC=$(wc -l < "$T/ci.txt")
echo "  本地清单 example 门 = $NL · ci.yml example 门 = $NC"

# ── 2 规模锚点 ──
[ "$NL" -ge 100 ] && ok "本地清单规模下限（≥100）" || bad "本地清单只 $NL 个 example 门 —— 提取器坏了？"
[ "$NC" -ge 100 ] && ok "ci.yml 规模下限（≥100）" || bad "ci.yml 只 $NC 个 example 门 —— 提取器坏了？"

# ── 3 双向差集 ──
comm -23 "$T/local.txt" "$T/ci.txt" > "$T/only_local.txt"
comm -13 "$T/local.txt" "$T/ci.txt" > "$T/only_ci.txt"

# ── 4 例外表（格式 `<路径>|<理由>`；必须仍然成立，否则判红）──
cat > "$T/exempt.txt" <<'EOF'
examples/m136_go_json_fidelity/verify.sh|CI 有意跳过（它需要 go；ci.yml 里有注释说明：该门由本地全量门覆盖）
examples/m117_realworld_defects/verify.sh|CI 独有：本地在 selfhost/m117_gates.sh（M117 专项运行器）里跑，不在 m116 全量清单内
examples/m138_regex_go_parity/verify.sh|CI 独有：第三份真值需要 go（本仓 CI 无 go 步骤 ⇒ 本地跑会自动 SKIP 并响亮打印原因）
examples/m67_multiarch/verify.sh|CI 独有：多架构 qemu 由 ci.yml 的 m67 job 专管（本机无齐全交叉工具链时不跑）
examples/m67_aarch64/verify.sh|CI 独有：aarch64 qemu，同上
EOF
cut -d'|' -f1 "$T/exempt.txt" | sort -u > "$T/exempt_paths.txt"

# 4a 漏注册：本地有、CI 没有、且不在例外表
comm -23 "$T/only_local.txt" "$T/exempt_paths.txt" > "$T/bad_local.txt"
if [ -s "$T/bad_local.txt" ]; then
    bad "以下门在本地清单里、ci.yml 里**没有**（漏注册 —— M235 缺陷 355 的同形）："
    sed 's/^/       /' "$T/bad_local.txt"
else
    ok "无漏注册（本地有 ⇒ CI 有，或已登记例外）"
fi

# 4b 反向缺口：CI 有、本地没有、且不在例外表
comm -23 "$T/only_ci.txt" "$T/exempt_paths.txt" > "$T/bad_ci.txt"
if [ -s "$T/bad_ci.txt" ]; then
    bad "以下门 ci.yml 里**有**、本地清单里没有（本地无处复跑 —— 只靠 CI 的话本地看不见）："
    sed 's/^/       /' "$T/bad_ci.txt"
else
    ok "无反向缺口（CI 有 ⇒ 本地有，或已登记例外）"
fi

# 4c **过期判据**：例外条目若已不在任何差集里 ⇒ 它已不成立 ⇒ 判红
EXPIRED=0; NEX=0
while IFS='|' read -r p reason; do
    [ -z "$p" ] && continue
    NEX=$((NEX+1))
    if ! grep -qxF "$p" "$T/only_local.txt" && ! grep -qxF "$p" "$T/only_ci.txt"; then
        bad "例外登记**已过期**：$p 现在两侧都有（或都没有）—— 原理由：$reason"
        EXPIRED=$((EXPIRED+1))
    fi
done < "$T/exempt.txt"
[ "$EXPIRED" = 0 ] && ok "例外表无过期条目（$NEX 条 · 全部仍然成立）"

# 4d 例外条目必须带理由
NOREASON=$(awk -F'|' 'NF<2 || length($2)<8 {n++} END{print n+0}' "$T/exempt.txt")
[ "$NOREASON" = 0 ] && ok "例外条目全部带理由（≥8 字）" || bad "$NOREASON 条例外缺理由"

# ── 5 **推广**：selfhost / tools / packaging 级「脚本门」同样双向（M247 补）──
#   为什么推广：M243 建立本守卫时只覆盖 `examples/*/verify.sh`，而本仓还有一批
#   `run <name> bash|python3 selfhost|tools|packaging/<script>` 形式的门 ——
#   它们**同样**会「清单加了、CI 忘了」（M235 缺陷 355 的形状）。
#   M247 实测：22 个脚本门里 **2 个**不在 ci.yml ⇒ 1 个补进 CI（本守卫自己）、
#   1 个登记例外（check_cross_ports.sh 在 CI 上必然全档 SKIP）。
grep -E '^[[:space:]]*run[[:space:]]' "$REG" \
  | grep -oE '(selfhost|tools|packaging)/[A-Za-z0-9_.-]+\.(sh|py)' | sort -u > "$T/s_local.txt"
: > "$T/s_ci.txt"
while read -r p; do
    grep -qF "$p" "$CI" && echo "$p" >> "$T/s_ci.txt"
done < "$T/s_local.txt"
sort -u "$T/s_ci.txt" -o "$T/s_ci.txt"
NSL=$(wc -l < "$T/s_local.txt")
echo "  本地清单脚本门 = $NSL"
[ "$NSL" -ge 15 ] && ok "脚本门规模下限（≥15）" || bad "只 $NSL 个脚本门 —— 提取器坏了？"

cat > "$T/s_exempt.txt" <<'EOF'
selfhost/check_cross_ports.sh|CI runner 无 /opt/muslcc-bin ⇒ 本门在 CI 上必然**全档 SKIP**（门自己会打印原因）；权威覆盖在 ci.yml 的 m67_multiarch job（那里带 qemu 真跑）
EOF
cut -d'|' -f1 "$T/s_exempt.txt" | sort -u > "$T/s_exempt_paths.txt"

comm -23 "$T/s_local.txt" "$T/s_ci.txt" > "$T/s_only_local.txt"
comm -23 "$T/s_only_local.txt" "$T/s_exempt_paths.txt" > "$T/s_bad.txt"
if [ -s "$T/s_bad.txt" ]; then
    bad "以下**脚本门**在清单里、ci.yml 里没有（漏注册 · M235 缺陷 355 同形）："
    sed 's/^/       /' "$T/s_bad.txt"
else
    ok "脚本门无漏注册（清单有 ⇒ CI 有，或已登记例外）"
fi

SEXP=0; NSEX=0
while IFS='|' read -r p reason; do
    [ -z "$p" ] && continue
    NSEX=$((NSEX+1))
    if ! grep -qxF "$p" "$T/s_only_local.txt"; then
        bad "脚本门例外**已过期**：$p 现在两边都有（或都没有）—— 原理由：$reason"
        SEXP=$((SEXP+1))
    fi
done < "$T/s_exempt.txt"
[ "$SEXP" = 0 ] && ok "脚本门例外无过期条目（$NSEX 条 · 仍然成立）"

echo
echo "结果：通过 $PASS / 失败 $FAIL"
[ "$FAIL" = 0 ] && { echo "GATE-REGISTRY-OK"; exit 0; } || { echo "GATE-REGISTRY-FAIL"; exit 1; }
