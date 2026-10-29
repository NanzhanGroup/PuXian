#!/usr/bin/env bash
# ============================================================
# examples/m278_release_reconcile/verify.sh —— M278 门
#     「发布通道对账」：把「tag ⇄ Release ⇄ 资产」判据化
# ------------------------------------------------------------
# 起因（2026-10-06 实测事故）：
#   `release.yml` 单通道 concurrency（cancel-in-progress:false）。GitHub 的语义是
#   「同组同时只允许一个 in-progress + **一个** pending」，后来的 pending 会**取消**先前的。
#   ⇒ 一次性推多个 tag（历史回填）时，先到的 Release run 被**静默取消**：
#     实测 v0.2.275 的 rpm 三个 job 从未跑；v0.2.261/263/264/265/271 **至今没有任何 Release**。
#     而**没有任何东西会告诉你**（CI 不报 · Tag Guard 不报 · 镜像不报）。
#
# 本门层次：
#   [1] 工具离线自证（30 断言：三档 fixture + 双向精确相等 + 豁免/过期/无效行 + --json + --rerun）
#   [2] 静态判据：缺陷事实与纪律写进脚本头与豁免表（防「知识只在提交信息里」）
#   [3] 豁免表纪律：每条必须有理由；理由为空 ⇒ 无效（不当豁免）
#   [4] 实网对账（**仅当有 GH_TOKEN**；没有则响亮 SKIP 并打印原因，不假装通过）
#   [5] 负控：本门**必须能判红** —— 用「豁免表塞进一个最新 tag」的副本跑，判 STALE-IGNORE
#   [6] 覆盖边界（如实登记）
# 用法：verify.sh [--neg-skip]      （--neg-skip 供 CI：负控在离线自证里，成本低，仍会跑）
# 退出码：0 通过 · 1 失败
# ============================================================
set -uo pipefail
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
NEG=1
for a in "$@"; do case "$a" in --neg-skip) NEG=0;; esac; done

P=0; F=0
ok(){ P=$((P+1)); printf '  ✅ %s\n' "$1"; }
ng(){ F=$((F+1)); printf '  ❌ %s\n' "$1"; }
# 缺陷 621（M354）：`printf ... | grep -q` + `set -o pipefail` = **假红装置**
#   现场（2026-10-29 全量门 shard2）：[2] 连报 2 条静态断言红，而同一份快照里 needle 明明在。
#   机理：grep -q 命中即退出，printf 还在写 → SIGPIPE(141) → pipefail 把整条管道判失败 → “不含”。
#   实测：14 并发跑本门 6 次假红，DBG 记到 pipestatus=「141 0」（grep 其实命中了）。
#   修法：herestring（无管道无 SIGPIPE）；红消息只带前 200 字，不再整份快照刷日志。
chkg(){ if grep -q -- "$3" <<<"$2"; then ok "$1"; else ng "$1（[${2:0:200}] 不含 [$3]）"; fi; }
chk(){ if [ "$2" = "$3" ]; then ok "$1"; else ng "$1（期望 [$3] 实得 [$2]）"; fi; }

TOOL=packaging/release_reconcile.sh
IGN=packaging/release_reconcile.ignore
# ── 缺陷 595（M339 补 2）：静态判据必须读**一个原子快照**，不能反复 cat 工作树 ──
#   现场（2026-10-27 12:1x 全量门 shard2）：[2] 第 4 条断言报红，而**同一份内容里 needle 明明在**
#     ❌ 只重跑 cancelled（failed 要人看）（[…  ⚠️ 只重跑 cancelled —— failed 可能是真失败 …] 不含 [只重跑 cancelled]）
#   同树单跑必绿（本条实测 ✅）。分片里 GATE_JOBS=2 ⇒ **同 worktree 有两个门并发**，
#   静态断言却对文件做了多次独立读取 ⇒ 任何第三方在这瞬间改这份文件都会被读成「内容缺失」。
#   ⇒ 统一改为：门开始时**一次**读入快照（优先 `git show HEAD:<path>`：与已提交内容对齐、不受工作树瞬态影响），
#     之后所有静态断言都判这份快照 —— 同一门内判定自洽，不再有「读到半份」的可能。
TOOL_SNAP=$(git show "HEAD:$TOOL" 2>/dev/null || cat "$TOOL")
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT

echo "=== M278 发布通道对账门 ==="

printf '\n[1] 工具离线自证（fixture · 不联网）\n'
if out=$(bash packaging/selftest_reconcile.sh 2>&1); then
    tail -2 <<<"$out" | sed 's/^/     /'
    ok "自证全绿：$(grep -oE '自证：[0-9]+ 通过 / [0-9]+ 失败' <<<"$out" | tail -1)"
else
    printf '%s\n' "$out" | tail -25 | sed 's/^/     /'
    ng "离线自证判红"
fi

printf '\n[2] 静态判据：缺陷事实与纪律在位（防知识丢失）\n'
chkg "脚本头记录单通道并发事实" "$TOOL_SNAP" "只允许一个 in-progress"
chkg "脚本头记录被取消的实测 tag" "$TOOL_SNAP" "v0.2.263/264/265/271"
chkg "脚本头写明「没有东西会告诉你」" "$TOOL_SNAP" "而没有任何东西会告诉你"
chkg "只重跑 cancelled（failed 要人看）" "$TOOL_SNAP" "只重跑 cancelled"
chkg "令牌只走环境变量" "$TOOL_SNAP" "必须来自环境变量"
chkg "豁免表写明「真缺陷不许进表」" "$(cat $IGN)" "真缺陷不许进表"
chkg "豁免表写明过期判据" "$(cat $IGN)" "STALE-IGNORE"
n=$(awk -F'\t' '!/^#/ && NF>0' "$IGN" | wc -l)
[ "$n" -ge 1 ] && ok "豁免表条目 $n 条（每条带理由）" || ng "豁免表为空"

printf '\n[2b] 装置自证：grep -q 不得经管道（缺陷 621 · SIGPIPE + pipefail = 假红）\n'
# 口径 14：前提断言先行。装置自证**不许依赖信号处置**（M354 第二遍全量门实测）：
#   分片环境里 SIGPIPE 被置 SIG_IGN，此时 `yes|grep -q` 得 1（yes 收 EPIPE 自行 rc=1，pipefail 取右端），
#   2s 前才在本机得 141 —— 同一行代码随环境漂移，装置本身就是假红源。
#   且 bash 非交互 shell 对「进入时已 ignore」的信号 `trap - PIPE` **恢复不了**（POSIX 规定），所以
#   自证分三件：① here-string 判据双向（命中/不命中）② 静态守卫（chkg 不许管道）③ SIGPIPE 处置**登记**（只报不判）。
t_hit=$(grep -q 'needle621' <<< 'xx needle621 xx' && echo hit || echo miss)
t_miss=$(grep -q 'absent621' <<< 'xx needle621 xx' || echo miss)
chk "自证 ①：here-string 判据命中（命中必须读成命中）" "$t_hit" "hit"
chk "自证 ①：here-string 判据不命中（不命中必须读成不命中）" "$t_miss" "miss"
if ! command -v yes >/dev/null 2>&1 || ! command -v grep >/dev/null 2>&1; then
    printf '  ⏭ [2c] PREMISE-FAIL：缺 yes/grep，跳过 SIGPIPE 处置登记（不算红也不算绿）\n'
else
    ( set -o pipefail; yes 2>/dev/null | grep -q y ); sp=$?
    case "$sp" in
        141) printf '  ℹ️ [2c] 本环境 SIGPIPE=默认 ⇒ 管道式 grep -q 会被判 141（621 假红机理可复现）\n';;
        1)   printf '  ℹ️ [2c] 本环境 SIGPIPE=ignored（分片 systemd-run 实测）⇒ 141 不复现；判据仍只用 here-string，不受影响\n';;
        *)   printf '  ⚠️ [2c] SIGPIPE 处置未知（yes|grep -q rc=%s）⇒ 登记待查，不影响本门判定\n' "$sp";;
    esac
fi
# 静态守卫：两处判据助手（本门 + 离线自证）都不许退回管道式
badg=""
for f in "$HERE/verify.sh" "$ROOT/packaging/selftest_reconcile.sh"; do
    ln=$(grep -m1 '^chkg()' "$f")
    case "$ln" in *printf*'| grep -q'*) badg="$badg $(basename "$(dirname "$f")")/$(basename "$f")";; esac
done
[ -z "$badg" ] && ok "判据助手 chkg 已不用管道喂 grep -q" || ng "chkg 仍是管道式（$badg）"

printf '\n[3] 豁免表纪律：理由为空的条目无效\n'
printf 'v9.9.9\t\n' > "$W/bad.ignore"
# fixture 必须**非空**（否则工具 rc=2「对账表为空」，到不了豁免解析这一步 —— 首版踩过）
mkdir -p "$W/fx"
cat > "$W/fx/releases.json" <<'JSON'
[{"tag_name":"v9.9.5","draft":false,"prerelease":false,"assets":[{"name":"puxian-9.9.5-a.tar.gz","size":1},{"name":"sha256sums.txt","size":1}]}]
JSON
cat > "$W/fx/runs.json" <<'JSON'
{"workflow_runs":[{"head_branch":"v9.9.5","head_sha":"5555555555555555555555555555555555555555","id":5,"status":"completed","conclusion":"success"}]}
JSON
o=$(GH_IGNORE="$W/bad.ignore" bash "$TOOL" --api-fixture "$W/fx" --limit 5 2>&1); rc=$?
chkg "报无效行" "$o" "无效行"; chk "无效行计入异常 ⇒ rc=1" "$rc" "1"
o2=$(GH_IGNORE=/dev/null bash "$TOOL" --api-fixture "$W/fx" --limit 5 2>&1); rc2=$?
chk "同一 fixture 无无效行时 rc=0（证明红来自那条无效行）" "$rc2" "0"

printf '\n[4] 实网对账（需 GH_TOKEN；无则响亮 SKIP）\n'
if [ -n "${GH_TOKEN:-}" ]; then
    if o=$(GH_TOKEN="$GH_TOKEN" GH_IGNORE="$IGN" bash "$TOOL" --limit 12 2>&1); then
        printf '%s\n' "$o" | tail -4 | sed 's/^/     /'
        ok "实网对账完成（rc=0）"
    else
        printf '%s\n' "$o" | tail -8 | sed 's/^/     /'
        ng "实网对账判红（有该有发布却没有的 tag）"
    fi
else
    echo "     ⏭ SKIP：未设置 GH_TOKEN —— 实网对账需要 GitHub API 令牌"
    echo "        本机：GH_TOKEN=\$(cat /data/pat.md) bash $0 ；CI：不设（本层按 SKIP 处理）"
fi

printf '\n[5] 负控：判据必须能判红（豁免了「最新 tag」⇒ STALE-IGNORE）\n'
mkdir -p "$W/st"; printf '[]\n' > "$W/st/releases.json"
cat > "$W/st/runs.json" <<'JSON'
{"workflow_runs":[{"head_branch":"v9.9.7","head_sha":"7777777777777777777777777777777777777777","id":7,"status":"completed","conclusion":"cancelled"}]}
JSON
printf 'v9.9.7\t负控：豁免一个最新 tag\n' > "$W/st.ignore"
o=$(GH_IGNORE="$W/st.ignore" bash "$TOOL" --api-fixture "$W/st" --limit 5 2>&1); rc=$?
chkg "判 STALE-IGNORE" "$o" "STALE-IGNORE"; chk "rc=1" "$rc" "1"

printf '\n[6] 覆盖边界（如实登记）\n'
cat <<'TXT'
     · 「该有发布却没有」的判据依赖 **GitHub Releases API**；API 不可达时 rc=2（响亮，不静默放行）
     · 豁免表只对「**确实没有 Release**」的判定生效；有 Release 但资产不全 ⇒ 仍判异常（不被豁免掩盖）
     · 只扫 `release.yml` 的 run；其它 workflow（CI / Tag Guard）不在面内
     · 「Release 存在但**内容**是坏的」不在面内（内容正确性由 make_release 的包内冒烟与
       真机冒烟脚本 realhost_smoke.sh 负责）
     · 历史回填 tag 的豁免是**判断**，不是事实：若日后要为其补发布，删豁免条目即恢复判红
TXT

echo
echo "M278 门结果：$P 通过 / $F 失败"
[ "$F" = 0 ] && echo "M278-VERIFY-OK" || echo "M278-VERIFY-FAIL"
exit "$([ "$F" = 0 ] && echo 0 || echo 1)"
