#!/usr/bin/env bash
# ============================================================
# packaging/selftest_reconcile.sh —— release_reconcile.sh 的**离线**自证（M278）
# ------------------------------------------------------------
# 全离线：用 fixture 目录喂 `--api-fixture`，**不联网、不碰真仓库**（CI 可跑）。
# fixture 由本脚本用 heredoc 现造在临时目录里 ⇒ 仓库里不留 fixture 文件，
# 也避免「新增 packaging/*.py/.sh 要两边注册」的注册面（M243 守卫只在
# registry 与 ci.yml **提到**的路径上做双向一致）。
#
# 层次：
#   [1] 好档（全有发布）        ⇒ rc=0
#   [2] 坏档（三种异常+豁免+在跑）⇒ 逐条判定**双向精确相等** + 计数
#   [3] 过期豁免（豁免了最新 tag）⇒ STALE-IGNORE
#   [4] 豁免表无效行（理由空）   ⇒ 计入异常、不当豁免
#   [5] 真实豁免表可解析          ⇒ 无理由为空
#   [6] --json 结构 + 计数一致
#   [7] --rerun 只碰 cancelled
#   [8] 用法/环境（未知选项 rc=2 · 无令牌 rc=2）
#   [9] 负控 A：抽掉豁免处理 ⇒ 豁免项必须判异常（判据有牙）
#   [10] 负控 B：判据自伤（异常计数恒 0）⇒ 坏档应变 OK（证明红来自比对）
#   [11] 规模下限 + 诊断走 stderr
# 退出码：0 全绿 · 1 有失败
# ============================================================
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SH=${SH:-$HERE/release_reconcile.sh}
REAL_IGN=${GH_IGNORE:-$HERE/release_reconcile.ignore}
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
P=0; F=0
ok(){ P=$((P+1)); printf '  ✅ %s\n' "$1"; }
ng(){ F=$((F+1)); printf '  ❌ %s\n' "$1"; }
chk(){ if [ "$2" = "$3" ]; then ok "$1"; else ng "$1（期望 [$3] 实得 [$2]）"; fi; }
chkg(){ if printf '%s' "$2" | grep -q -- "$3"; then ok "$1"; else ng "$1（[$2] 不含 [$3]）"; fi; }

# ── 现造 fixture（自洽：每档自带 ignore 表）──
mkfix(){
  local d="$W/fix/$1"; mkdir -p "$d"
  cat > "$d/releases.json"; cat > "$d/runs.json"; cat > "$d/ignore"
}
mkfix good <<'JSON'
[{"tag_name":"v9.9.3","draft":false,"prerelease":false,"assets":[{"name":"puxian-9.9.3-abc.tar.gz","size":1},{"name":"puxian-bootstrap-aarch64-v9.9.3.tar.gz","size":1},{"name":"sha256sums.txt","size":1}]},
 {"tag_name":"v9.9.2","draft":false,"prerelease":false,"assets":[{"name":"puxian-9.9.2-abc.tar.gz","size":1},{"name":"sha256sums.txt","size":1}]}]
JSON
cat > "$W/fix/good/runs.json" <<'JSON'
{"workflow_runs":[{"head_branch":"v9.9.3","head_sha":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","id":9003,"status":"completed","conclusion":"success"},
 {"head_branch":"v9.9.2","head_sha":"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb","id":9002,"status":"completed","conclusion":"success"}]}
JSON
printf '# 空豁免表\n' > "$W/fix/good/ignore"

mkdir -p "$W/fix/bad"
cat > "$W/fix/bad/releases.json" <<'JSON'
[{"tag_name":"v9.9.5","draft":false,"prerelease":false,"assets":[{"name":"puxian-9.9.5-abc.tar.gz","size":1},{"name":"puxian-bootstrap-aarch64-v9.9.5.tar.gz","size":1},{"name":"sha256sums.txt","size":1}]},
 {"tag_name":"v9.9.2","draft":false,"prerelease":false,"assets":[{"name":"puxian-9.9.2-abc.tar.gz","size":1}]}]
JSON
cat > "$W/fix/bad/runs.json" <<'JSON'
{"workflow_runs":[{"head_branch":"v9.9.5","head_sha":"cccccccccccccccccccccccccccccccccccccccc","id":9005,"status":"completed","conclusion":"success"},
 {"head_branch":"v9.9.4","head_sha":"dddddddddddddddddddddddddddddddddddddddd","id":9004,"status":"completed","conclusion":"cancelled"},
 {"head_branch":"v9.9.3","head_sha":"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee","id":9003,"status":"completed","conclusion":"cancelled"},
 {"head_branch":"v9.9.2","head_sha":"ffffffffffffffffffffffffffffffffffffffff","id":9002,"status":"completed","conclusion":"success"},
 {"head_branch":"v9.9.1","head_sha":"1111111111111111111111111111111111111111","id":9001,"status":"in_progress","conclusion":null}]}
JSON
printf 'v9.9.3\t历史回填 tag（fixture）· 只为账目 · 不需要发布\n' > "$W/fix/bad/ignore"

mkdir -p "$W/fix/stale"
printf '[]\n' > "$W/fix/stale/releases.json"
cat > "$W/fix/stale/runs.json" <<'JSON'
{"workflow_runs":[{"head_branch":"v9.9.7","head_sha":"7777777777777777777777777777777777777777","id":9007,"status":"completed","conclusion":"cancelled"}]}
JSON
printf 'v9.9.7\t过期豁免（fixture）：最新 tag 不该被豁免\n' > "$W/fix/stale/ignore"

run(){ GH_IGNORE="$1/ignore" bash "$SH" --api-fixture "$1" "${@:2}" 2>&1; }

echo
echo "=== release_reconcile 离线自证（M278）==="
printf '\n【1】好档：全部有发布 ⇒ rc=0\n'
o=$(run "$W/fix/good" --limit 10); rc=$?
chk "rc" "$rc" "0"; chkg "结论行" "$o" "RELEASE-RECONCILE-OK"; chkg "计数" "$o" "异常 0 个"

printf '\n【2】坏档：三种异常 + 一种豁免 + 一种在跑\n'
o=$(run "$W/fix/bad" --limit 10); rc=$?
chk "rc" "$rc" "1"; chkg "结论行" "$o" "RELEASE-RECONCILE-WARN"
exp="v9.9.5=OK
v9.9.4=NO-RELEASE-CANCELLED
v9.9.3=KNOWN-NO-RELEASE
v9.9.2=ASSETS-INCOMPLETE
v9.9.1=NO-RELEASE-INPROGRESS"
got=$(printf '%s\n' "$o" | awk -F'[ ]+' '/^v9\./{print $1"="$NF}' | sort)
chk "逐条判定（双向精确相等）" "$got" "$(printf '%s\n' "$exp" | sort)"
chkg "异常计数=2" "$o" "异常 2 个"
chkg "豁免计数=1" "$o" "豁免 1 条"
if printf '%s' "$o" | grep -qE 'v9\.9\.1.*in_progress'; then ok "在跑状态可见（不是裸 None）"; else ng "在跑状态未体现"; fi
if printf '%s' "$o" | grep -q 'None'; then ng "输出里出现 None（应显示 status）"; else ok "无 None 占位"; fi

printf '\n【3】过期豁免：豁免表里写着「最新 tag」⇒ 判 STALE-IGNORE（异常）\n'
o=$(run "$W/fix/stale" --limit 10); rc=$?
chk "rc" "$rc" "1"; chkg "判 STALE-IGNORE" "$o" "STALE-IGNORE"

printf '\n【4】豁免表无效行（理由为空）⇒ 计入异常，不当豁免\n'
printf 'v9.9.4\t\n' > "$W/ign_empty"
o=$(GH_IGNORE="$W/ign_empty" bash "$SH" --api-fixture "$W/fix/bad" --limit 10 2>&1); rc=$?
chk "rc" "$rc" "1"; chkg "报无效行" "$o" "无效行"; chkg "v9.9.4 仍判 CANCELLED" "$o" "NO-RELEASE-CANCELLED"

printf '\n【5】真实豁免表（仓库里那份）解析\n'
if [ -f "$REAL_IGN" ]; then
  nbad=$(awk -F'\t' '!/^#/ && NF>0 { if ($2=="" || $2 ~ /^[[:space:]]*$/) print }' "$REAL_IGN" | wc -l)
  chk "无理由为空的条目" "$nbad" "0"
  n=$(awk -F'\t' '!/^#/ && NF>0' "$REAL_IGN" | wc -l); [ "$n" -ge 1 ] && ok "条目数 $n ≥ 1" || ng "豁免表为空"
  chkg "豁免表头写明纪律" "$(cat "$REAL_IGN")" "真缺陷不许进表"
else ng "找不到豁免表 $REAL_IGN"; fi

printf '\n【6】--json 可解析且计数一致\n'
o=$(run "$W/fix/bad" --limit 10 --json)
if printf '%s' "$o" | python3 -c '
import sys,json
d=json.load(sys.stdin)
assert d["bad"]==2, ("bad",d["bad"])
assert any(i["verdict"]=="KNOWN-NO-RELEASE" for i in d["items"])
assert len(d["items"])==5, len(d["items"])
print("JSONOK")' 2>"$W/json.err" | grep -q JSONOK; then ok "--json 结构正确"; else ng "--json：$(cat "$W/json.err")"; fi

printf '\n【7】--rerun 只碰 cancelled（fixture 档不下发请求）\n'
o=$(run "$W/fix/bad" --limit 10 --rerun)
chkg "报已重跑" "$o" "已重跑 cancelled"
if printf '%s' "$o" | grep -q 'v9.9.3(fixture)'; then ng "不应重跑豁免项"; else ok "豁免项不重跑"; fi

printf '\n【8】用法与环境\n'
bash "$SH" --bogus >/dev/null 2>&1; chk "未知选项 rc" "$?" "2"
( env -u GH_TOKEN bash "$SH" --limit 1 >/dev/null 2>&1 ); chk "无 GH_TOKEN 且无 fixture ⇒ rc=2" "$?" "2"

printf '\n【9】负控 A：抽掉豁免处理 ⇒ 豁免项必须判异常（判据有牙）\n'
sed 's/^        verdict = .STALE-IGNORE.*$/        pass/; s/^        verdict = .KNOWN-NO-RELEASE.*$/        pass/' "$SH" > "$W/no_ign.sh"
if ! diff -q "$SH" "$W/no_ign.sh" >/dev/null; then
  o=$(GH_IGNORE="$W/fix/bad/ignore" bash "$W/no_ign.sh" --api-fixture "$W/fix/bad" --limit 10 2>&1)
  chkg "不再豁免" "$o" "NO-RELEASE-CANCELLED"
  chkg "异常计数升到 3" "$o" "异常 3 个"
else ng "负控 A 未改动脚本（锚点失效）"; fi

printf '\n【10】负控 B：判据自伤（异常计数恒 0）⇒ 坏档应变 OK（证明红来自比对）\n'
sed 's|^BAD=\$(awk -F.*$|BAD=0|' "$SH" > "$W/selfharm.sh"
if ! diff -q "$SH" "$W/selfharm.sh" >/dev/null; then
  o=$(GH_IGNORE="$W/fix/bad/ignore" bash "$W/selfharm.sh" --api-fixture "$W/fix/bad" --limit 10 2>&1); rc=$?
  chk "自伤后 rc" "$rc" "0"; chkg "自伤后结论" "$o" "RELEASE-RECONCILE-OK"
else ng "负控 B 未改动脚本（锚点失效）"; fi

printf '\n【11】规模下限与通道\n'
L=$(wc -l < "$SH"); [ "$L" -ge 100 ] && ok "脚本行数 $L ≥ 100" || ng "脚本过短 $L"
bash "$SH" --help >/dev/null 2>&1 && ok "--help rc=0" || ng "--help rc≠0"
if bash "$SH" --bogus 2>/dev/null | grep -q .; then ng "诊断不该走 stdout"; else ok "诊断走 stderr"; fi

echo
echo "release_reconcile 自证：$P 通过 / $F 失败"
[ "$F" = 0 ] && echo "RECONCILE-SELFTEST-OK" || echo "RECONCILE-SELFTEST-FAIL"
exit "$([ "$F" = 0 ] && echo 0 || echo 1)"
