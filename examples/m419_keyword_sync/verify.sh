#!/usr/bin/env bash
# ============================================================
# M419 门 · **保留字表单一事实源**（缺陷 841 / 842）
# ------------------------------------------------------------
# 病灶（2026-11-18 实测 · HEAD b38da5e）：
#   ① docs/spec.md §2.4 列 35 个关键字，实现有 41 个 —— 多列 1 个假的（type），
#      漏列 7 个真的（and / or / not / None / extern / try / catch）；
#      §5.8 与设计决策表第 7 条仍写「明确不做 try/catch/throw」，而 M392 已三轨实现。
#      ⇒ 照文档写 px 直接报 E2001（AI 写代码会系统性写错），文档与实现**相反**。
#   ② 12 个 .px 各自手抄一份 KEYWORDS（selfhost 7 + tools 5），零判据看管：
#      加一个关键字要改 12 处，漏一处就是「编译轨认、解释轨不认」的裂脑。
#   ③ M392 把 try/catch 变保留字是**静默 breaking change**（无 edition 标注、CHANGELOG 无记）。
#
# 本门三层（每层都能独立判红）：
#   [1] 自测：check_keyword_sync.sh --selftest 9 条（含 6 条负控 + 2 条前提自证 + 行指纹陷阱）
#   [2] 真仓正控：名单 ⇄ 实际持有者双向对账 + 12 份键集合一致 + spec §2.4 逐字一致
#   [3] 真仓镜像负控：把真文件复制成夹具后注入三种扰动，各必须判红
#       N1 多一份未登记持有者 · N2 某副本缺一个键 · N3 spec 少一个关键字
#       每条都带「扰动真发生」的前提自证 —— 否则红可能来自夹具本身坏了
#   环境问题（名单/spec 缺失、样本面为 0）一律 rc=3，绝不判绿。
#
# 用法：bash examples/m419_keyword_sync/verify.sh [--selftest]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
set -uo pipefail
SELF_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$SELF_DIR/../.." && pwd)
CK="$ROOT/selfhost/check_keyword_sync.sh"

pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }
say() { echo; echo "── $* ──"; }

[ -f "$CK" ] || { echo "❌ 判据实现缺失：$CK" >&2; exit 3; }
[ -x "$CK" ] || chmod +x "$CK" 2>/dev/null || true

say "[1] 自测（判据自己有没有牙）"
if bash "$CK" --selftest > /tmp/m419_selftest.log 2>&1; then
    ok "自测通过（$(grep -oE 'pass=[0-9]+ fail=[0-9]+' /tmp/m419_selftest.log | tail -1)）"
else
    bad "自测失败"; tail -15 /tmp/m419_selftest.log | sed 's/^/     /'
fi

say "[2] 真仓正控（三方对账：实现 ⇄ spec ⇄ 名单）"
if bash "$CK" > /tmp/m419_real.log 2>&1; then
    ok "真仓一致（$(grep -oE '名单 [0-9]+ 份 / 实际持有 [0-9]+ 份' /tmp/m419_real.log | head -1)；$(grep -oE '实现键数 = [0-9]+' /tmp/m419_real.log)）"
else
    bad "真仓不一致 —— 保留字表已漂移（见 /tmp/m419_real.log）"
    grep -E '❌|漏列|多列|不一致' /tmp/m419_real.log | head -8 | sed 's/^/     /'
fi
# 前提自证：这份"绿"来自 12 份真文件与 41 键，而不是空集
n_own=$(bash -c "grep -vc '^[[:space:]]*#' '$ROOT/selfhost/keywords_owners.txt' || true")
if [ "${n_own:-0}" -ge 10 ] && grep -q '实现键数 = 4[0-9]' /tmp/m419_real.log; then
    ok "前提自证：名单 $n_own 份 · $(grep -oE '实现键数 = [0-9]+' /tmp/m419_real.log)（扫描面不是空的）"
else
    bad "前提自证失败：名单 $n_own 份（判据可能已空转）"
fi

say "[3] 真仓镜像负控（复制真文件后注入扰动，必须判红）"
T=$(mktemp -d /tmp/m419_fix.XXXXXX)
trap 'rm -rf "$T"' EXIT
mkdir -p "$T/selfhost" "$T/tools" "$T/docs"
while IFS= read -r f; do
    case "$f" in ''|'#'*) continue ;; esac
    mkdir -p "$T/$(dirname "$f")"
    cp "$ROOT/$f" "$T/$f"
done < "$ROOT/selfhost/keywords_owners.txt"
cp "$ROOT/selfhost/keywords_owners.txt" "$T/selfhost/keywords_owners.txt"
cp "$ROOT/docs/spec.md" "$T/docs/spec.md"

if bash "$CK" --repo "$T" > /tmp/m419_fix_base.log 2>&1; then
    ok "夹具基线正控：复制真文件后判绿（防「夹具天然红」冒充负控生效）"
else
    bad "夹具基线不正：复制真文件后仍红 ⇒ 下面三条负控的红不可信"
    tail -8 /tmp/m419_fix_base.log | sed 's/^/     /'
fi

# N1 多一份未登记持有者
cp "$T/selfhost/lexer.px" "$T/tools/pxnew.px"
if bash "$CK" --repo "$T" > /tmp/m419_n1.log 2>&1; then
    bad "N1 负控不成立：多一份未登记持有者仍判绿（名单对账无牙）"
else
    if grep -q '未登记持有者' /tmp/m419_n1.log; then
        ok "N1 负控成立：未登记持有者 tokens/pxnew.px ⇒ $(grep -oE '未登记持有者 [0-9]+ 份' /tmp/m419_n1.log | head -1)"
    else
        bad "N1 判红但原因不是「未登记持有者」（红错地方）"
    fi
fi
{ [ -f "$T/tools/pxnew.px" ] && grep -q 'let KEYWORDS' "$T/tools/pxnew.px"; }; ok_or_no=$?
[ "$ok_or_no" -eq 0 ] && ok "N1 前提自证：扰动文件确实持有 KEYWORDS 表" || bad "N1 前提自证失败：扰动文件没造出来"
rm -f "$T/tools/pxnew.px"

# N2 某副本缺一个键（改 bx_cli 的 KEYWORDS 行：删 "try": "try", ）
python3 - "$T/selfhost/bc_cli.px" <<'PY'
import re, sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
new = re.sub(r'"try":\s*"try",\s*', '', s, count=1)
open(p, 'w', encoding='utf-8').write(new)
PY
if bash "$CK" --repo "$T" > /tmp/m419_n2.log 2>&1; then
    bad "N2 负控不成立：副本缺键仍判绿（键集合一致性无牙）"
else
    if grep -q '键集合不一致' /tmp/m419_n2.log; then
        ok "N2 负控成立：bc_cli.px 缺 try 键 ⇒ 判红"
    else
        bad "N2 判红但原因不是「键集合不一致」"
    fi
fi
if ! grep -q '"try":' "$T/selfhost/bc_cli.px"; then
    ok "N2 前提自证：夹具里 try 键确已删除（扰动真发生）"
else
    bad "N2 前提自证失败：扰动没生效（python 锚点失效）⇒ 本轮结论不可信"
fi
cp "$ROOT/selfhost/bc_cli.px" "$T/selfhost/bc_cli.px"   # 复原

# N3 spec 少一个关键字
python3 - "$T/docs/spec.md" <<'PY'
import re, sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
m = re.search(r'### 2\.4 关键字（保留字）\n```\n(.*?)\n```', s, re.S)
assert m, 'spec §2.4 代码块锚点失效'
body = m.group(1).replace('catch', 'ctch')
s = s[:m.start(1)] + body + s[m.end(1):]
open(p, 'w', encoding='utf-8').write(s)
PY
if bash "$CK" --repo "$T" > /tmp/m419_n3.log 2>&1; then
    bad "N3 负控不成立：spec 少一个关键字仍判绿（文档对账无牙）"
else
    if grep -q 'spec 漏列' /tmp/m419_n3.log; then
        ok "N3 负控成立：spec 把 catch 写成 ctch ⇒ $(grep -oE 'spec (漏列|多列) [0-9]+ 个' /tmp/m419_n3.log | tr '\n' ' ')"
    else
        bad "N3 判红但原因不是「spec 漏列」"
    fi
fi
if grep -q 'ctch' "$T/docs/spec.md"; then
    ok "N3 前提自证：spec 夹具里 catch 确已改名"
else
    bad "N3 前提自证失败：扰动没生效 ⇒ 本轮结论不可信"
fi

say "[4] 环境问题不判绿"
cp "$ROOT/docs/spec.md" "$T/docs/spec.md"        # 先把 N3 的扰动复原，隔离出唯一变量
rm -f "$T/selfhost/keywords_owners.txt"
bash "$CK" --repo "$T" > /tmp/m419_env.log 2>&1; rc_env=$?
if [ "$rc_env" -eq 3 ] && ! grep -q 'KEYWORD-SYNC=OK' /tmp/m419_env.log; then
    ok "无名单文件 ⇒ rc=3 且不判 OK（不是「没东西可查就算绿」）"
else
    bad "环境问题处理不当：rc=$rc_env（期望 3，且不得出现 KEYWORD-SYNC=OK）"
fi

echo
echo "m419_keyword_sync: pass=$pass fail=$fail"
if [ "$fail" -eq 0 ]; then echo "M419-KEYWORD-SYNC-OK"; exit 0; fi
echo "M419-KEYWORD-SYNC-FAIL fails=$fail"; exit 1
