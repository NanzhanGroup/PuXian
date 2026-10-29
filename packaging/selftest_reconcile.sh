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
# 缺陷 621（M354）：`printf ... | grep -q` + `set -o pipefail` = 假红装置
#   grep -q 命中即退出，printf 还在写 ⇒ SIGPIPE(141) ⇒ pipefail 把整条管道判失败
#   ⇒ 内容明明在，却报「不含」。实测：14 并发跑 m278，6 次假红，pipestatus=「141 0」。
#   ⇒ 统一改 herestring（无管道、无 SIGPIPE）；红消息只带前 200 字（原先整份快照刷进日志）
chkg(){ if grep -q -- "$3" <<<"$2"; then ok "$1"; else ng "$1（[${2:0:200}] 不含 [$3]）"; fi; }

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
# 第二源（缺陷 586/587 修复后新增的两个 fixture 文件）：git tag 列表 + releases/latest
printf '[{"name":"v9.9.3"},{"name":"v9.9.2"}]\n' > "$W/fix/good/tags.json"
printf '{"tag_name":"v9.9.3"}\n' > "$W/fix/good/latest.json"

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
printf '[{"name":"v9.9.5"},{"name":"v9.9.4"},{"name":"v9.9.3"},{"name":"v9.9.2"},{"name":"v9.9.1"}]\n' > "$W/fix/bad/tags.json"
printf '{"tag_name":"v9.9.5"}\n' > "$W/fix/bad/latest.json"

mkdir -p "$W/fix/stale"
printf '[]\n' > "$W/fix/stale/releases.json"
cat > "$W/fix/stale/runs.json" <<'JSON'
{"workflow_runs":[{"head_branch":"v9.9.7","head_sha":"7777777777777777777777777777777777777777","id":9007,"status":"completed","conclusion":"cancelled"}]}
JSON
printf 'v9.9.7\t过期豁免（fixture）：最新 tag 不该被豁免\n' > "$W/fix/stale/ignore"
printf '[{"name":"v9.9.7"}]\n' > "$W/fix/stale/tags.json"
printf '{"tag_name":"v9.9.7"}\n' > "$W/fix/stale/latest.json"

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
if grep -qE 'v9\.9\.1.*in_progress' <<<"$o"; then ok "在跑状态可见（不是裸 None）"; else ng "在跑状态未体现"; fi
if grep -q 'None' <<<"$o"; then ng "输出里出现 None（应显示 status）"; else ok "无 None 占位"; fi

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
if grep -q 'v9.9.3(fixture)' <<<"$o"; then ng "不应重跑豁免项"; else ok "豁免项不重跑"; fi

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
_so=$(bash "$SH" --bogus 2>/dev/null)
# 不用管道：若真把诊断打到 stdout，管道式会因 SIGPIPE 判 141 ⇒ 走 else ⇒ **假绿**（缺陷 621 的另一面）
if grep -q . <<<"$_so"; then ng "诊断不该走 stdout"; else ok "诊断走 stderr"; fi

printf '\n【12】缺陷 586：tag 在 git tags 里、但既无 Release 又无 run ⇒ 必须判 NO-RELEASE-NORUN\n'
mkdir -p "$W/fix/orphan"
cat > "$W/fix/orphan/releases.json" <<'JSON'
[{"tag_name":"v9.9.9","draft":false,"prerelease":false,"assets":[{"name":"puxian-9.9.9-abc.tar.gz"},{"name":"puxian-bootstrap-aarch64-v9.9.9.tar.gz"},{"name":"sha256sums.txt"}]},
 {"tag_name":"v9.9.7","draft":false,"prerelease":false,"assets":[{"name":"puxian-9.9.7-abc.tar.gz"},{"name":"sha256sums.txt"}]}]
JSON
cat > "$W/fix/orphan/runs.json" <<'JSON'
{"workflow_runs":[{"head_branch":"v9.9.9","head_sha":"9999999999999999999999999999999999999999","id":9909,"status":"completed","conclusion":"success"},
 {"head_branch":"v9.9.7","head_sha":"7777777777777777777777777777777777777777","id":9907,"status":"completed","conclusion":"success"}]}
JSON
# 关键：v9.9.8 只存在于 git tag 列表（无 Release、无 run）—— 这正是 2026-10-26 事故的形态；
#       v9.9.8-m55 是带后缀的开发 tag ⇒ **不该进要求集**（作用域判据）
printf '[{"name":"v9.9.9"},{"name":"v9.9.8"},{"name":"v9.9.7"},{"name":"v9.9.8-m55"}]\n' > "$W/fix/orphan/tags.json"
printf '{"tag_name":"v9.9.9"}\n' > "$W/fix/orphan/latest.json"
printf '# 空豁免表\n' > "$W/fix/orphan/ignore"
o=$(run "$W/fix/orphan" --limit 10); rc=$?
chk "rc" "$rc" "1"
chkg "v9.9.8 判 NO-RELEASE-NORUN" "$o" "v9.9.8"
if grep -q 'v9.9.8 .*NO-RELEASE-NORUN' <<<"$o"; then ok "v9.9.8 = NO-RELEASE-NORUN"; else ng "v9.9.8 判定不符"; fi
if grep -q 'v9.9.8-m55' <<<"$o"; then ng "带后缀的开发 tag 进了要求集（作用域失效）"; else ok "带后缀开发 tag 不进要求集"; fi
chkg "异常计数=1" "$o" "异常 1 个"
# 负控：拿掉第二源（去掉 tags.json）⇒ v9.9.8 消失、异常归零
#   ⇒ 证明这条红**来自新数据源**，不是别的东西顺手报出来的
cp -r "$W/fix/orphan" "$W/fix/orphan_nosrc"; rm -f "$W/fix/orphan_nosrc/tags.json"
o=$(run "$W/fix/orphan_nosrc" --limit 10); rc=$?
if [ "$rc" = 0 ] && ! grep -q 'v9\.9\.8' <<<"$o"; then
  ok "负控：无第二源时看不见 v9.9.8 且 rc=0 ⇒ 红确实来自第二源（判据有牙）"
else
  ng "负控失效：无 tags.json 仍报 v9.9.8 或 rc≠0（rc=$rc）"
fi

printf '\n【13】缺陷 587：releases/latest ≠ 最高版本 tag ⇒ 必须判 LATEST-STALE\n'
cp -r "$W/fix/orphan" "$W/fix/lateststale"; rm -f "$W/fix/lateststale/tags.json"
printf '[{"name":"v9.9.9"},{"name":"v9.9.7"}]\n' > "$W/fix/lateststale/tags.json"
printf '{"tag_name":"v9.9.7"}\n' > "$W/fix/lateststale/latest.json"   # latest 被旧版本顶掉
o=$(run "$W/fix/lateststale" --limit 10); rc=$?
chk "rc" "$rc" "1"; chkg "报 LATEST-STALE" "$o" "LATEST-STALE"; chkg "异常计数=1" "$o" "异常 1 个"
o=$(run "$W/fix/lateststale" --limit 10 --json)
if printf '%s' "$o" | python3 -c 'import sys,json; d=json.load(sys.stdin); assert d["latest_stale"]==1 and d["latest"]=="v9.9.7" and d["latest_expected"]=="v9.9.9" and d["bad"]==1; print("OK")' 2>"$W/ls.err" | grep -q OK; then
  ok "--json 暴露 latest / latest_expected / latest_stale"
else ng "--json latest 字段：$(cat "$W/ls.err")"; fi
# 正控：latest = 最高版本 tag ⇒ 不得报
cp -r "$W/fix/lateststale" "$W/fix/latestok"
printf '{"tag_name":"v9.9.9"}\n' > "$W/fix/latestok/latest.json"
o=$(run "$W/fix/latestok" --limit 10); rc=$?
if [ "$rc" = 0 ] && ! grep -q 'LATEST-STALE' <<<"$o"; then ok "正控：latest 正确时不报（不造假红）"; else ng "正控失败：latest 正确仍报（rc=$rc）"; fi
# 负控：去掉 latest.json ⇒ 能力降级，不判红（但必须在 stderr 说清）
rm -f "$W/fix/latestok/latest.json"
o=$(GH_IGNORE="$W/fix/latestok/ignore" bash "$SH" --api-fixture "$W/fix/latestok" --limit 10 2>&1); rc=$?
if [ "$rc" = 0 ] && grep -q '\[587\] 拿不到 releases/latest' <<<"$o"; then
  ok "负控：无 latest 数据 ⇒ 降级并响亮告知（不判红也不静默）"
else ng "负控失败（rc=$rc）：$(printf '%s' "$o" | grep 587 || echo 无提示)"; fi

echo
echo "release_reconcile 自证：$P 通过 / $F 失败"
[ "$F" = 0 ] && echo "RECONCILE-SELFTEST-OK" || echo "RECONCILE-SELFTEST-FAIL"
exit "$([ "$F" = 0 ] && echo 0 || echo 1)"
