#!/usr/bin/env bash
# ============================================================
# packaging/selftest_latest_guard.sh —— latest_guard.sh 的**离线**自证（M335 · 缺陷 587/588）
# ------------------------------------------------------------
# 全离线：fixture 目录喂 `--api-fixture`，不联网、不碰真仓库（CI 可跑）。
# fixture 由本脚本现造（仓库不留 fixture 文件，避免新增注册面）。
#
# 层次：
#   [1] 一致档                 ⇒ rc=0
#   [2] latest 被旧 tag 顶掉    ⇒ rc=1 + 文案指名两者（这是 2026-10-26 的真实现场）
#   [3] 作用域：带后缀开发 tag 不参与比较 ⇒ 不得误判
#   [4] 无 latest（一个 Release 都没有）⇒ rc=0 + 响亮降级（不判红也不静默）
#   [5] 无「纯数字 tag」        ⇒ rc=0 + 降级提示
#   [6] --json 结构
#   [7] 用法/环境（未知选项 · 无令牌无 fixture）
#   [8] 负控 A：判据自伤（比较恒等）⇒ 坏档必须变绿（证明红来自比较）
#   [9] 负控 B：把期望值算成「最低版本 tag」⇒ 一致档必须变红（证明期望值这一侧有牙）
# 退出码：0 全绿 · 1 有失败
# ============================================================
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SH=${SH:-$HERE/latest_guard.sh}
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
P=0; F=0
ok(){ P=$((P+1)); printf '  ✅ %s\n' "$1"; }
ng(){ F=$((F+1)); printf '  ❌ %s\n' "$1"; }
chk(){ if [ "$2" = "$3" ]; then ok "$1"; else ng "$1（期望 [$3] 实得 [$2]）"; fi; }
# 缺陷 621（M354）：`printf ... | grep -q` + `set -o pipefail` = 假红装置（SIGPIPE 141 被读成「不含」）。
#   实测现场：本文件 [2b] 断言「报 PENDING」，输出里明明有 PENDING（分片负载下 24/25）。
#   修法：here-string（无管道无 SIGPIPE）；反向断言尤其危险（SIGPIPE 会造**假绿**）。
chkg(){ if grep -q -- "$3" <<<"$2"; then ok "$1"; else ng "$1（[${2:0:200}] 不含 [$3]）"; fi; }
chkn(){ if grep -q -- "$3" <<<"$2"; then ng "$1（[${2:0:200}] 不该含 [$3]）"; else ok "$1"; fi; }

mk(){ mkdir -p "$W/$1"; }

echo
echo "=== latest_guard 离线自证（M335）==="

# ── [1] 一致 ──
mk ok1
printf '[{"name":"v9.9.9"},{"name":"v9.9.8"},{"name":"v9.9.7"}]\n' > "$W/ok1/tags.json"
printf '{"tag_name":"v9.9.9"}\n' > "$W/ok1/latest.json"
printf '\n【1】一致档（latest = 最高版本 tag）⇒ rc=0\n'
o=$(bash "$SH" --api-fixture "$W/ok1" 2>&1); rc=$?
chk "rc" "$rc" "0"; chkg "文案" "$o" "releases/latest = v9.9.9"

# ── [2] latest 被旧 tag 顶掉 ──
mk bad1
printf '[{"name":"v9.9.9"},{"name":"v9.9.8"},{"name":"v9.9.7"}]\n' > "$W/bad1/tags.json"
printf '{"tag_name":"v9.9.7"}\n' > "$W/bad1/latest.json"
printf '\n【2】latest 被旧 tag 顶掉（2026-10-26 真实现场）⇒ rc=1\n'
# M339：STALE 的前提是「期望版本的 Release **已经存在**」；fixture 里用 expected_release.json 表达
printf '{"tag_name":"v9.9.9","id":999}\n' > "$W/bad1/expected_release.json"
o=$(bash "$SH" --api-fixture "$W/bad1" 2>&1); rc=$?
chk "rc" "$rc" "1"; chkg "报 LATEST-STALE" "$o" "LATEST-STALE"
chkg "指名实到版本" "$o" "v9.9.7"; chkg "指名期望版本" "$o" "v9.9.9"
chkg "说清后果" "$o" "旧树"

printf '\n【2b】期望版本的 Release **还不存在**（发布中）⇒ 不判红，判 PENDING\n'
cp -r "$W/bad1" "$W/pending"; rm -f "$W/pending/expected_release.json"
o=$(bash "$SH" --api-fixture "$W/pending" 2>&1); rc=$?
chk "rc=0（发布窗口内不该假红）" "$rc" "0"; chkg "报 PENDING" "$o" "PENDING"
chkg "指向对账工具" "$o" "release_reconcile"

# ── [3] 作用域：带后缀的开发 tag 不进要求集 ──
mk scope
printf '[{"name":"v9.9.9"},{"name":"v9.9.9-m55"},{"name":"v9.9.8"},{"name":"v0.2.0-m100"}]\n' > "$W/scope/tags.json"
printf '{"tag_name":"v9.9.9"}\n' > "$W/scope/latest.json"
printf '\n【3】作用域：v9.9.9-m55 / v0.2.0-m100 这类开发 tag 不参与比较\n'
o=$(bash "$SH" --api-fixture "$W/scope" 2>&1); rc=$?
chk "rc=0（开发 tag 若参与会把期望值算高 ⇒ 假红）" "$rc" "0"
chkn "期望值不是开发 tag" "$o" "latest = v9.9.9-m55"

# ── [4] 无 latest ──
mk nolatest
printf '[{"name":"v9.9.9"}]\n' > "$W/nolatest/tags.json"
printf '\n【4】一个 Release 都没有（拿不到 latest）⇒ rc=0 + 响亮降级\n'
o=$(bash "$SH" --api-fixture "$W/nolatest" 2>&1); rc=$?
chk "rc" "$rc" "0"; chkg "降级提示" "$o" "不判红也不静默"

# ── [5] 无纯数字 tag ──
mk notags
printf '[{"name":"v9.9.9-m1"}]\n' > "$W/notags/tags.json"
printf '{"tag_name":"v9.9.9-m1"}\n' > "$W/notags/latest.json"
printf '\n【5】没有纯数字版本 tag ⇒ rc=0 + 降级提示\n'
o=$(bash "$SH" --api-fixture "$W/notags" 2>&1); rc=$?
chk "rc" "$rc" "0"; chkg "降级提示" "$o" "跳过"

# ── [6] --json ──
printf '\n【6】--json 结构\n'
o=$(bash "$SH" --api-fixture "$W/bad1" --json 2>/dev/null)
if jout=$(python3 -c 'import sys,json; d=json.load(sys.stdin); assert d["verdict"]=="STALE" and d["expected"]=="v9.9.9" and d["latest"]=="v9.9.7"; print("OK")' <<<"$o" 2>"$W/j.err") && grep -q OK <<<"$jout"; then
  ok "--json 字段正确（verdict/expected/latest）"
else ng "--json：$(cat "$W/j.err")"; fi
o=$(bash "$SH" --api-fixture "$W/ok1" --json 2>/dev/null)
if printf '%s' "$o" | python3 -c 'import sys,json; d=json.load(sys.stdin); assert d["verdict"]=="OK"; print("OK")' 2>/dev/null | grep -q OK; then ok "--json OK 档"; else ng "--json OK 档"; fi

# ── [7] 用法/环境 ──
printf '\n【7】用法与环境\n'
bash "$SH" --bogus >/dev/null 2>&1; chk "未知选项 rc" "$?" "2"
( env -u GH_TOKEN bash "$SH" >/dev/null 2>&1 ); chk "无令牌无 fixture ⇒ rc=2" "$?" "2"
bash "$SH" --help >/dev/null 2>&1 && ok "--help rc=0" || ng "--help rc≠0"

# ── [8] 负控 A：比较恒等 ⇒ 坏档变绿 ──
printf '\n【8】负控 A：判据自伤（比较恒等）⇒ 坏档必须变绿\n'
sed 's/^if \[ "\$CURRENT" = "\$EXPECTED" \]; then$/if true; then/' "$SH" > "$W/selfharm.sh"
if ! diff -q "$SH" "$W/selfharm.sh" >/dev/null; then
  o=$(bash "$W/selfharm.sh" --api-fixture "$W/bad1" 2>&1); rc=$?
  chk "自伤后 rc" "$rc" "0"; chkg "自伤后结论" "$o" "releases/latest ="
else ng "负控 A 未改动脚本（锚点失效）"; fi

# ── [9] 负控 B：期望值侧有牙（取最低版本当期望）──
printf '\n【9】负控 B：期望值算成「最低版本 tag」⇒ 一致档必须变红\n'
sed 's/if best is None or key(n) > key(best): best = n/if best is None or key(n) < key(best): best = n/' "$SH" > "$W/lowexp.sh"
if ! diff -q "$SH" "$W/lowexp.sh" >/dev/null; then
  # ⚠️ 必须让「算错的期望值」也有 Release 标记，否则会走 PENDING（发布中）分支而不是 STALE
  cp -r "$W/ok1" "$W/okneg"; printf '{"tag_name":"v9.9.7"}\n' > "$W/okneg/expected_release.json"
  o=$(bash "$W/lowexp.sh" --api-fixture "$W/okneg" 2>&1); rc=$?
  chk "rc" "$rc" "1"; chkg "报 STALE" "$o" "LATEST-STALE"
else ng "负控 B 未改动脚本（锚点失效）"; fi

# ── [10] 装置自证：判据读的是内容，不走管道（缺陷 621/622）──
#   口径 18：只断言「判据在两种取值上给出正确结果」，不断言信号/调度现象的数值；
#   SIGPIPE 处置只登记不判红（分片 systemd-run 里是 SIG_IGN，此时 yes|grep -q 得 1 而非 141）。
printf '\n【10】装置自证：判据不走管道（621/622）\n'
h1=$(grep -q 'needle621' <<< 'xx needle621 xx' && echo hit || echo miss)
h2=$(grep -q 'absent621' <<< 'xx needle621 xx' || echo miss)
chk "here-string 判据：命中读成命中" "$h1" "hit"
chk "here-string 判据：不命中读成不命中" "$h2" "miss"
_dl=$(grep -n '^chkg()\|^chkn()' "${BASH_SOURCE[0]}"); _bad=$(grep -c '| grep -q' <<< "$_dl")
chk "静态守卫：chkg/chkn 不许用管道喂 grep -q" "${_bad:-0}" "0"
if command -v yes >/dev/null 2>&1; then
  ( set -o pipefail; yes 2>/dev/null | grep -q y ); _sp=$?
  case "$_sp" in 141) printf '  ℹ️ SIGPIPE=默认（141 可复现）\n';; 1) printf '  ℹ️ SIGPIPE=ignored（分片常态，141 不复现；本判据不用管道，不受影响）\n';; *) printf '  ⚠️ SIGPIPE 处置未知 rc=%s（登记待查）\n' "$_sp";; esac
fi

echo
echo "latest_guard 自证：$P 通过 / $F 失败"
[ "$F" = 0 ] && echo "LATEST-GUARD-SELFTEST-OK" || echo "LATEST-GUARD-SELFTEST-FAIL"
exit "$([ "$F" = 0 ] && echo 0 || echo 1)"
