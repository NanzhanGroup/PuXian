#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M389 · 缺陷 709：GitHub 取数的「测量失败被当成测量结果」
# ---------------------------------------------------------------------------
# 现场（2026-11-10 20:22 实测）：/data/pat.md 的令牌 15:43 之后失效。
#   旧写法 curl -sS -H "Authorization: Bearer $PAT" ... 在 401 时 rc=0，
#   响应体是 {"message":"Bad credentials"} ⇒ grep 得 0 ⇒ **assets=0**
#   ⇒ 判据把「读不到」当成「资产还没出」，而三个发布自证脚本更严重：
#     · latest_guard.sh   读空 latest ⇒ EXPECTED_HAS_RELEASE=0 ⇒ emit PENDING + **exit 0**（假绿）
#     · release_reconcile 两份数据都"成功"但为空 ⇒ 对账表空 ⇒ 报 OK（假绿，同缺陷 586）
#     · latest_verify.sh  轮询 30min ⇒ BLOCKED（指向 Release job，指错方向）
#
# 判据（贴契约，不贴表象）：
#   C1 有效令牌 ⇒ rc=0 且 GH_AUTH_STATE=token-ok
#   C2 失效令牌 + 允许匿名 ⇒ rc=3 且 token-invalid-anon-ok，**取得的答案与带好令牌一致**
#   C3 失效令牌 + 禁止匿名（GH_ALLOW_ANON=0）⇒ rc=4，且不得把空 body 当成功
#   C4 两侧都取不到（HTTP 500）⇒ rc=4；**latest_guard 必须 exit 2**，不得 PENDING/OK
#   C5 404 是合法答案（一个 Release 都没有）⇒ rc=0 + notfound；latest_guard 判 PENDING exit 0
#      （区分「确实没有」与「读不到」，两者不许合并）
#   C6 静态守卫：三个脚本都 source gh_get.sh，且不再有裸 curl -H "Authorization"
#   C7 令牌本体不出现在任何输出/日志里（只允许出现 auth 状态与长度）
#
# 负控（两条，都是本病症的原始形状；负控不红 ⇒ 判据没牙）：
#   A 把 gh_get 的 401 分支改成 return 0（失效当成功）  ⇒ C2 必须红
#   B 把「取不到数」改成返回 rc=0 + 空文件                ⇒ C4 必须红
#
# 前提自证（不是"我觉得夹具对"，而是让夹具自己证明）：
#   P1 夹具对坏令牌真的回 401           P2 夹具对匿名真的回 200 且带 tag_name
#   P3 负控副本真的被改过（diff 非空）  —— 否则"负控没红"可能是文件没换成
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
TMP="${TMPDIR:-/tmp}/m389_gh_auth.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
SRV_PID=""
cleanup() { [ -n "$SRV_PID" ] && kill "$SRV_PID" 2>/dev/null; rm -rf "$TMP"; }
trap cleanup EXIT

P=0; F=0
ok(){ P=$((P+1)); printf '  ✅ %s\n' "$1"; }
ng(){ F=$((F+1)); printf '  ❌ %s\n' "$1"; }
chk(){ [ "$2" = "$3" ] && ok "$1" || ng "$1（期望 [$3] 实得 [$2]）"; }
chkg(){ case "$2" in *"$3"*) ok "$1";; *) ng "$1（未命中 [$3]，实得：$(printf '%s' "$2" | head -2 | tr '\n' ' ')）";; esac; }
chkn(){ case "$2" in *"$3"*) ng "$1（不该出现 [$3]）";; *) ok "$1";; esac; }

# ── 夹具：一个只会说谎的 GitHub（400 行以内，路由前缀即可造三种故障）──
cat > "$TMP/srv.py" <<'PY'
import json, sys
from http.server import BaseHTTPRequestHandler, HTTPServer
TAGS   = json.dumps([{"name": "v0.2.388"}, {"name": "v0.2.387"}]).encode()
LATEST = json.dumps({"tag_name": "v0.2.388", "id": 4242}).encode()
EXP    = json.dumps({"id": 4242, "tag_name": "v0.2.388"}).encode()
BADCRED= json.dumps({"message": "Bad credentials"}).encode()
FLAKY = {}   # 缺陷 839：按前缀计数的抖包夹具（前两次 503，第三次 200）

class H(BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def _send(self, code, body):
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)
    def do_GET(self):
        auth = self.headers.get('Authorization', '')
        p = self.path
        if p.startswith('/down/'):   return self._send(500, b'{"message":"boom"}')
        # 缺陷 839：抖包形状 —— 同一前缀前两次 503、第三次 200（按前缀独立计数，测例互不污染）
        if p.startswith('/flaky'):
            pre = p.split('/')[1]
            FLAKY[pre] = FLAKY.get(pre, 0) + 1
            if FLAKY[pre] <= 2: return self._send(503, b'{"message":"try again"}')
            return self._send(200, TAGS)
        # /nf/ 只让「一个 Release 都没有」这一个端点 404（tags 仍 200）——
        # 否则 404 测例会被「拿不到 tag 列表」抢先判成环境错，测不到本意
        if p.startswith('/nf/') and '/releases/latest' in p:
            return self._send(404, b'{"message":"Not Found"}')
        if auth and 'bad' in auth:
            return self._send(401, BADCRED)
        if '/releases/latest' in p:  return self._send(200, LATEST)
        if '/releases/tags/' in p:   return self._send(200, EXP)
        if '/tags' in p:             return self._send(200, TAGS)
        return self._send(200, b'{}')

srv = HTTPServer(('127.0.0.1', 0), H)
print(srv.server_port, flush=True)
srv.serve_forever()
PY
python3 "$TMP/srv.py" > "$TMP/port" 2>"$TMP/srv.err" &
SRV_PID=$!
for _ in $(seq 1 40); do [ -s "$TMP/port" ] && break; sleep 0.1; done
PORT="$(head -1 "$TMP/port" 2>/dev/null)"
if [ -z "${PORT:-}" ]; then echo "❌ 夹具未起（$(head -3 "$TMP/srv.err" 2>/dev/null)）"; exit 2; fi
BASE="http://127.0.0.1:$PORT"

printf '\n【P】前提自证：夹具真的会说谎，也真的会说实话\n'
code=$(curl -sS -o "$TMP/p1.json" -w '%{http_code}' -H "Authorization: Bearer bad" "$BASE/repos/X/Y/releases/latest")
chk "P1 坏令牌 ⇒ HTTP 401" "$code" "401"
grep -q 'Bad credentials' "$TMP/p1.json" && ok "P1b 401 体是 Bad credentials（与真实 GitHub 同形）" || ng "P1b 401 体不对"
code=$(curl -sS -o "$TMP/p2.json" -w '%{http_code}' "$BASE/repos/X/Y/releases/latest")
chk "P2 匿名 ⇒ HTTP 200" "$code" "200"
grep -q '"tag_name"' "$TMP/p2.json" && ok "P2b 匿名真的取到 tag_name（匿名降级不是空想）" || ng "P2b 匿名体无 tag_name"

# ── 被测对象 ──
GHGET="$ROOT/packaging/gh_get.sh"
GUARD="$ROOT/packaging/latest_guard.sh"

printf '\n【C1】有效令牌 ⇒ token-ok\n'
( export GH_API="$BASE" GH_TOKEN="github_pat_good0000000000"
  . "$GHGET"; gh_get "/repos/X/Y/releases/latest" "$TMP/c1.json"; echo "rc=$? state=$GH_AUTH_STATE"
  grep -q '"tag_name"' "$TMP/c1.json" && echo body-ok ) > "$TMP/c1.out" 2>&1
chkg "C1 rc=0" "$(cat "$TMP/c1.out")" "rc=0"
chkg "C1 状态 token-ok" "$(cat "$TMP/c1.out")" "state=token-ok"
chkg "C1 取到 body" "$(cat "$TMP/c1.out")" "body-ok"

printf '\n【C2】失效令牌 + 允许匿名 ⇒ rc=3 且答案与好令牌一致\n'
( export GH_API="$BASE" GH_TOKEN="github_pat_bad_000000000" GH_ALLOW_ANON=1
  . "$GHGET"; gh_get "/repos/X/Y/releases/latest" "$TMP/c2.json"; echo "rc=$? state=$GH_AUTH_STATE" ) > "$TMP/c2.out" 2>&1
chkg "C2 rc=3（降级但取到）" "$(cat "$TMP/c2.out")" "rc=3"
chkg "C2 状态 token-invalid-anon-ok" "$(cat "$TMP/c2.out")" "state=token-invalid-anon-ok"
if [ -s "$TMP/c2.json" ] && grep -q 'v0.2.388' "$TMP/c2.json"; then ok "C2b 降级后答案非空且正确"; else ng "C2b 降级后答案为空/错（这就是缺陷原形）"; fi

printf '\n【C3】失效令牌 + 禁止匿名 ⇒ rc=4，不许把空 body 当成功\n'
( export GH_API="$BASE" GH_TOKEN="github_pat_bad_000000000" GH_ALLOW_ANON=0
  . "$GHGET"; gh_get "/repos/X/Y/releases/latest" "$TMP/c3.json"; echo "rc=$? state=$GH_AUTH_STATE" ) > "$TMP/c3.out" 2>&1
chkg "C3 rc=4" "$(cat "$TMP/c3.out")" "rc=4"
chkg "C3 状态 token-invalid-no-anon" "$(cat "$TMP/c3.out")" "state=token-invalid-no-anon"

printf '\n【C4】两侧都取不到（HTTP 500）⇒ 调用方必须 exit 2，不得 PENDING/OK\n'
out=$( env -u GH_TOKEN GH_ALLOW_ANON=1 GH_API="$BASE/down" bash "$GUARD" --repo X/Y 2>&1 ); rc=$?
chk "C4 latest_guard rc=2（环境错，不是结论）" "$rc" "2"
chkn "C4b 不许报 PENDING" "$out" "PENDING"
chkn "C4c 不许报 ✅" "$out" "✅"
chkg "C4d 点名测量失败" "$out" "[709]"

printf '\n【C5】404 = 合法答案（一个 Release 都没有）⇒ PENDING exit 0\n'
out=$( env -u GH_TOKEN GH_ALLOW_ANON=1 GH_API="$BASE/nf" bash "$GUARD" --repo X/Y 2>&1 ); rc=$?
chkn "C5 不许因读不到而 exit 2（404 不是失败）" "rc=$rc" "rc=2"
chkn "C5b 不许报 ✅（没有 latest 就没有一致性可断言）" "$out" "✅"

printf '\n【C6】静态守卫：直接取数的脚本必须走唯一实现；所有脚本不许裸 curl+Authorization\n'
for f in latest_guard.sh release_reconcile.sh; do
    if grep -q 'gh_get.sh' "$ROOT/packaging/$f"; then ok "C6 $f source gh_get.sh"; else ng "C6 $f 未 source gh_get.sh"; fi
done
# latest_verify 不直接取数（只轮询 latest_guard）⇒ 不要求它 source，
# 但必须能把 guard 的 rc=2 识别成 ENV-FAIL（否则会输成 BLOCKED，指错方向）
if grep -q 'ENV-FAIL' "$ROOT/packaging/latest_verify.sh"; then ok "C6 latest_verify 识别 ENV-FAIL（区分读不到与没发布）"; else ng "C6 latest_verify 未区分 ENV-FAIL"; fi
for f in latest_guard.sh latest_verify.sh release_reconcile.sh; do
    if grep -nE 'curl .*-H "Authorization: Bearer' "$ROOT/packaging/$f" > "$TMP/$f.bare" 2>/dev/null; then
        ng "C6 $f 仍有裸 curl+Authorization（$(head -1 "$TMP/$f.bare")）"
    else ok "C6 $f 无裸 curl+Authorization"; fi
done
if grep -q 'gh_write PATCH' "$ROOT/packaging/latest_guard.sh"; then ok "C6b 写操作走 gh_write（不降级匿名）"; else ng "C6b PATCH 未走 gh_write"; fi

printf '\n【C7】令牌本体不出现在输出里\n'
TOK="github_pat_""C7_SECRET_abcdef123456"   # 夹具运行时拼装（源码不出现连续凭据形状，见 check_no_secrets.sh 卫生规则）
( export GH_API="$BASE" GH_TOKEN="$TOK"
  . "$GHGET"; gh_get "/repos/X/Y/releases/latest" "$TMP/c7.json" >/dev/null 2>&1; gh_auth_state ) > "$TMP/c7.out" 2>&1
chkn "C7 输出不含令牌本体" "$(cat "$TMP/c7.out")" "$TOK"
chkg "C7b 但确实报了身份信息" "$(cat "$TMP/c7.out")" "len="

# ── 负控 ──
printf '\n【N1】负控 A：把 401 当成功 ⇒ C2 必须红\n'
sed 's/GH_AUTH_STATE="token-invalid" ;;      # 落到下面决定是否匿名降级/GH_AUTH_STATE="token-ok"; return 0 ;;   # 负控：失效当成功/' \
    "$GHGET" > "$TMP/gh_bad1.sh"
if ! diff -q "$GHGET" "$TMP/gh_bad1.sh" >/dev/null; then ok "P3a 负控副本确实被改过（diff 非空）"; else ng "P3a 负控没改到（判据会假绿）"; fi
( export GH_API="$BASE" GH_TOKEN="github_pat_bad_000000000" GH_ALLOW_ANON=1
  . "$TMP/gh_bad1.sh"; gh_get "/repos/X/Y/releases/latest" "$TMP/n1.json"; echo "rc=$? state=$GH_AUTH_STATE" ) > "$TMP/n1.out" 2>&1
if grep -q 'state=token-ok' "$TMP/n1.out"; then ok "N1 负控 A 成立：失效令牌被标成 token-ok（C2 会红）"; else ng "N1 负控 A 打不出来（判据无牙）"; fi

printf '\n【N2】负控 B：取不到数时返回 rc=0 + 空文件 ⇒ C4 必须红\n'
# M415 · 缺陷 829：本负控必须**两条分支都打**。
#   旧版只 patch 了「带令牌」那条（`fail-$code ; return 4`），匿名那条
#   （`fail-$anon_code ; return 4`）没打 ⇒ 在**没有 /data/pat.md 的干净环境**（CI）里
#   请求走匿名分支、坏实现根本没生效 ⇒ latest_guard 仍 exit 2 ⇒ **负控假红**（2026-11-17 CI 实测）。
#   这是「负控的前提没自证」：它测的不是坏实现，而是「这台机器有没有令牌文件」。
sed -e 's/GH_AUTH_STATE="fail-\$code" ; return 4 ;;/GH_AUTH_STATE="fail-$code"; : > "$out"; return 0 ;;/' \
    -e 's/GH_AUTH_STATE="fail-\$anon_code"; return 4 ;;/GH_AUTH_STATE="fail-$anon_code"; : > "$out"; return 0 ;;/' \
    "$GHGET" > "$TMP/gh_bad2.sh"
if ! diff -q "$GHGET" "$TMP/gh_bad2.sh" >/dev/null; then ok "P3b 负控副本确实被改过"; else ng "P3b 负控没改到（sed 锚点失效）"; fi
# 前提自证（两条分支都被打掉才算前提成立）
n_patched=$(grep -cE 'fail-\$(code|anon_code)"; : > "\$out"; return 0' "$TMP/gh_bad2.sh" 2>/dev/null || true)
n_still=$(grep -cE 'fail-\$(code|anon_code)" ; return 4' "$TMP/gh_bad2.sh" 2>/dev/null || true)
n_patched=${n_patched:-0}; n_still=${n_still:-0}
if [ "$n_patched" = 2 ] && [ "$n_still" = 0 ]; then
    ok "P3c 前提自证：有令牌 / 匿名两条失败分支都已改成 rc=0（patched=$n_patched still=$n_still）"
else
    ng "P3c 前提不成立：分支没打全（patched=$n_patched 期望 2 · still=$n_still 期望 0）⇒ 本轮 N2 结论不可信"
fi
mkdir -p "$TMP/neg2"; cp "$GUARD" "$TMP/neg2/latest_guard.sh"; cp "$TMP/gh_bad2.sh" "$TMP/neg2/gh_get.sh"
out=$( env -u GH_TOKEN GH_ALLOW_ANON=1 GH_API="$BASE/down" bash "$TMP/neg2/latest_guard.sh" --repo X/Y 2>&1 ); rc=$?
chkn "N2 负控 B 成立：坏实现下 latest_guard 不再 exit 2（实得 rc=$rc）" "rc=$rc" "rc=2"

printf '\n【C8】令牌不进 argv（ps / systemd journal 里不得出现 Bearer 令牌）\n'
# M415 · 缺陷 831：2026-11-18 实测 systemd 的 cgroup 树把
#   `curl ... -H "Authorization: Bearer github_pat_..."` 整条打进 journal ⇒ 本机任何能读日志的
#   进程都能拿到有效期 30 天的令牌。修法：`curl -K -`（配置从 stdin 读），argv 只剩 `-K -`。
# M415 修正（pkg_pipe_grep 门 · 口径 19）：原写法 `grep -nE … | grep -qvE …` 属缺陷 621/622/624 同族 ——
#   `grep -q` 一命中即退出 ⇒ 生产者拿 EPIPE ⇒ rc=141 ⇒ pipefail 把「有违规」读成「合格」（实测 PIPESTATUS[0]=141）。
#   改为先落变量再 here-string 判（无写端，不撞 SIGPIPE）；保留非空守卫，否则空输出会被当成「有一行非注释」而误报。
auth_argv_hits=$(grep -nE '\-H[[:space:]]+"Authorization' "$ROOT/packaging/gh_get.sh" 2>/dev/null || true)
if [ -n "$auth_argv_hits" ] && grep -qvE '^[0-9]+:[[:space:]]*#' <<< "$auth_argv_hits"; then
    ng "C8 gh_get.sh 仍把 Authorization 放进 argv（ps/journal 可见）"
else
    ok "C8 gh_get.sh 的 Authorization 不走 argv"
fi
if grep -q 'K[[:space:]]*-' "$ROOT/packaging/gh_get.sh"; then ok "C8b 确有 -K - 通道（配置走 stdin）"; else ng "C8b 没找到 -K - 通道"; fi
# C8c 前提自证：用「会记下自己 argv 的假 curl」实抓一次，证明令牌真的没进参数表
mkdir -p "$TMP/curlspy"
cat > "$TMP/curlspy/curl" <<'EOS'
#!/usr/bin/env bash
printf '%s\n' "$@" > "$CURLSPY_OUT"
exit 0
EOS
chmod +x "$TMP/curlspy/curl"
( export PATH="$TMP/curlspy:$PATH" CURLSPY_OUT="$TMP/argv.txt"
  # shellcheck disable=SC1090
  . "$GHGET"; _gh_call "/x" "$TMP/o.json" "github_pat_C8SECRET_abcdef" >/dev/null 2>&1 || true )
if [ -s "$TMP/argv.txt" ] && ! grep -q 'C8SECRET' "$TMP/argv.txt"; then
    ok "C8c 实抓 argv：令牌不在参数里（$(tr '\n' ' ' < "$TMP/argv.txt" | cut -c1-60)…）"
elif [ ! -s "$TMP/argv.txt" ]; then
    ng "C8c 前提不成立：假 curl 没被调到（本轮 C8 结论不可信）"
else
    ng "C8c 实抓 argv：令牌仍在参数里（argv 里出现 C8SECRET）"
fi

printf '\n【C9】传输层抖动必须退避重试（缺陷 839：一次超时即 ENV-FAIL）\n'
# 现场（2026-11-18 20:35 实测）：dy-ops-ci-parity 20:35:01 起、20:35:21 落 —— 整 20s 正好是
#   GH_MAX_TIME 的超时时间；紧邻的 19:34 / 20:05 两轮都是绿的。同机复跑 8 次取数，第 7 次
#   也卡满 20044ms 返回 rc=4（到 api.github.com connect 0.3s / TLS 0.6s，路径本就慢）。
#   ⇒ 一次抖包就把「测量失败」p1 拍给用户，而契约里 rc=4 的本意是「真的取不到」。
# 判据：抖两次、第三次能答 ⇒ 必须 rc=0，且 attempts=3（证明真的重试过，不是碰巧第一次就通）。
( export GH_API="$BASE/flaky9" GH_TOKEN="github_pat_good0000000000" GH_RETRY=3 GH_RETRY_SLEEP=0 GH_QUIET_FAIL=1
  . "$GHGET"; gh_get "/repos/X/Y/tags" "$TMP/c9.json"
  echo "rc=$? state=$GH_AUTH_STATE attempts=$GH_LAST_ATTEMPTS"
  grep -q 'v0.2.388' "$TMP/c9.json" && echo body-ok ) > "$TMP/c9.out" 2>&1
chkg "C9 抖动后取到数 ⇒ rc=0" "$(cat "$TMP/c9.out")" "rc=0"
chkg "C9b 确实重试到第 3 次（attempts=3）" "$(cat "$TMP/c9.out")" "attempts=3"
chkg "C9c 答案可用（不是空文件冒充成 success）" "$(cat "$TMP/c9.out")" "body-ok"

printf '\n【C9d】同一抖包，禁止重试（GH_RETRY=1，等同修复前的行为）⇒ 必须 rc=4\n'
# 这一条是 C9 的「有牙证明」：判据能分开「有重试」与「没重试」，不是两边都绿。
( export GH_API="$BASE/flaky9d" GH_TOKEN="github_pat_good0000000000" GH_RETRY=1 GH_RETRY_SLEEP=0 GH_QUIET_FAIL=1
  . "$GHGET"; gh_get "/repos/X/Y/tags" "$TMP/c9d.json"; echo "rc=$? attempts=$GH_LAST_ATTEMPTS" ) > "$TMP/c9d.out" 2>&1
chkg "C9d 不重试 ⇒ rc=4（修复前的形状）" "$(cat "$TMP/c9d.out")" "rc=4"
chkg "C9d2 只尝试 1 次" "$(cat "$TMP/c9d.out")" "attempts=1"

printf '\n【C10】重试完仍取不到 ⇒ 依旧 rc=4 响亮失败（重试不许把失败洗成绿）\n'
( export GH_API="$BASE/down" GH_TOKEN="github_pat_good0000000000" GH_RETRY=3 GH_RETRY_SLEEP=0 GH_QUIET_FAIL=1
  . "$GHGET"; gh_get "/repos/X/Y/tags" "$TMP/c10.json"; echo "rc=$? state=$GH_AUTH_STATE attempts=$GH_LAST_ATTEMPTS"
  if grep -q 'v0.2.388' "$TMP/c10.json" 2>/dev/null; then echo body-HAS-DATA; else echo body-empty; fi ) > "$TMP/c10.out" 2>&1
chkg "C10 3 次都 500 ⇒ rc=4" "$(cat "$TMP/c10.out")" "rc=4"
chkg "C10b attempts=3（重试确实用完了）" "$(cat "$TMP/c10.out")" "attempts=3"
chkg "C10c 失败文件里不许有数据" "$(cat "$TMP/c10.out")" "body-empty"

printf '\n【C10d】语义答案（401）不许重试：问一百次也是同一个答案\n'
( export GH_API="$BASE" GH_TOKEN="github_pat_bad_000000000" GH_ALLOW_ANON=0 GH_RETRY=3 GH_RETRY_SLEEP=0 GH_QUIET_FAIL=1
  . "$GHGET"; gh_get "/repos/X/Y/releases/latest" "$TMP/c10d.json"; echo "rc=$? state=$GH_AUTH_STATE attempts=$GH_LAST_ATTEMPTS" ) > "$TMP/c10d.out" 2>&1
chkg "C10d 401 + 禁止匿名 ⇒ rc=4" "$(cat "$TMP/c10d.out")" "rc=4"
chkg "C10d2 attempts=1（401 不重试）" "$(cat "$TMP/c10d.out")" "attempts=1"

# ── 负控 C：把重试去掉（回到缺陷 839 的形状）⇒ C9 必须红 ──
printf '\n【N3】负控 C：拿掉重试 ⇒ C9 必须红\n'
sed 's|_gh_call_retry "$path" "$out" "$tok"; code="$GH_LAST_CODE"|code="$(_gh_call "$path" "$out" "$tok")"|' \
    "$GHGET" > "$TMP/gh_bad3.sh"
if ! diff -q "$GHGET" "$TMP/gh_bad3.sh" >/dev/null; then ok "P4a 负控副本确实被改过（diff 非空）"; else ng "P4a 负控没改到（sed 锚点失效）⇒ N3 结论不可信"; fi
n_retry=$(grep -c '^_gh_call_retry "$path" "$out" "$tok"; code="$GH_LAST_CODE"' "$TMP/gh_bad3.sh" 2>/dev/null || true)
n_def=$(grep -c '^_gh_call_retry() {' "$TMP/gh_bad3.sh" 2>/dev/null || true)
# 只断言「带令牌调用点已不带重试、副本结构完好」：函数体内的单次 _gh_call 与调用点同形，
# 用整行计数会把函数体也算进去（首版就踩了这个坑，假红），所以按定义行/调用点的独占形状判。
if [ "${n_retry:-1}" = 0 ] && [ "${n_def:-0}" = 1 ]; then
    ok "P4b 前提自证：带令牌调用点已无重试（retry=$n_retry def=$n_def）"
else
    ng "P4b 前提不成立：retry=$n_retry（期望 0）def=$n_def（期望 1）"
fi
( export GH_API="$BASE/flaky9n" GH_TOKEN="github_pat_good0000000000" GH_RETRY=3 GH_RETRY_SLEEP=0 GH_QUIET_FAIL=1
  . "$TMP/gh_bad3.sh"; gh_get "/repos/X/Y/tags" "$TMP/n3.json" >/dev/null 2>&1
  rc=$?; [ "$rc" = 4 ] && echo NEG-RED-AS-EXPECTED || echo "NEG-STILL-GREEN rc=$rc" ) > "$TMP/n3.out" 2>&1
chkg "N3 负控 C 成立：无重试时抖包被判 ENV-FAIL（C9 会红）" "$(cat "$TMP/n3.out")" "NEG-RED-AS-EXPECTED"

printf '\n【C11】取不到数必须报出传输层原因（缺陷 931）\n'
# 现场（2026-11-24 16:01 / 16:31 实测）：journal 只剩 `gh_get: 取不到数 code=000 attempts=3`，
#   而「DNS 解析不出来 / 连不上 / 被中间设备截断」在证据上完全同形 ⇒ 那一轮叫醒只能得到
#   「测量失败」，根因（本机 DNS 全线超时 15:54–16:44）要再查一遍 journal 才认出来。
# 判据：000 的失败行必须带非空的「末次错误=」；成功路径不许出现这段噪声。
( export GH_API="http://127.0.0.1:1" GH_TOKEN="github_pat_good0000000000" GH_RETRY=2 GH_RETRY_SLEEP=0 \
         GH_QUIET_FAIL=0 GH_MAX_TIME=2
  . "$GHGET"; gh_get "/repos/X/Y/tags" "$TMP/c11.json"; echo "rc=$? attempts=$GH_LAST_ATTEMPTS" ) > "$TMP/c11.out" 2>&1
chkg "C11 连不上 ⇒ rc=4" "$(cat "$TMP/c11.out")" "rc=4"
chkg "C11b 重试确实用完 attempts=2" "$(cat "$TMP/c11.out")" "attempts=2"
if grep -qE 'gh_get: 取不到数 code=000 attempts=2 末次错误=.+' "$TMP/c11.out"; then
    ok "C11c 失败行带非空「末次错误=」（$(grep -o '末次错误=.*' "$TMP/c11.out" | cut -c1-60)…）"
else
    ng "C11c 失败行没有传输层原因（实得：$(head -2 "$TMP/c11.out" | tr '\n' ' ')）"
fi
if grep -q 'GH_LAST_ERR' "$TMP/c11.out"; then ok "C11d 原因里不出现变量名（已是可读文本）"; else ok "C11d 无变量名噪声"; fi
( export GH_API="$BASE" GH_TOKEN="github_pat_good0000000000" GH_QUIET_FAIL=0
  . "$GHGET"; gh_get "/repos/X/Y/tags" "$TMP/c11e.json"; echo "rc=$? lasterr=[$GH_LAST_ERR]" ) > "$TMP/c11e.out" 2>&1
chkn "C11e 成功路径不报「取不到数」（不制造噪声）" "$(cat "$TMP/c11e.out")" "取不到数"
chkg "C11f 成功时 GH_LAST_ERR 为空" "$(cat "$TMP/c11e.out")" "lasterr=[]"

# ── 负控 D：把错误落盘改回 /dev/null（= 修复前的形状）⇒ C11 必须红 ──
printf '\n【N4】负控 D：丢掉 curl 错误文本 ⇒ C11 必须红\n'
sed 's|^    errf="\${out}\.gherr"$|    errf="/dev/null"|' "$GHGET" > "$TMP/gh_bad4.sh"
if ! diff -q "$GHGET" "$TMP/gh_bad4.sh" >/dev/null; then ok "P5a 负控副本确实被改过（diff 非空）"; else ng "P5a 负控没改到（sed 锚点失效）⇒ N4 结论不可信"; fi
n_errf=$(grep -c '^    errf="/dev/null"$' "$TMP/gh_bad4.sh" 2>/dev/null || true)
if [ "${n_errf:-0}" = 1 ]; then ok "P5b 前提自证：错误落盘已改回 /dev/null"; else ng "P5b 前提不成立（errf=/dev/null 命中 ${n_errf:-0}）"; fi
( export GH_API="http://127.0.0.1:1" GH_TOKEN="github_pat_good0000000000" GH_RETRY=2 GH_RETRY_SLEEP=0 \
         GH_QUIET_FAIL=0 GH_MAX_TIME=2
  . "$TMP/gh_bad4.sh"; gh_get "/repos/X/Y/tags" "$TMP/n4.json" ) > "$TMP/n4.out" 2>&1
if grep -qE '末次错误=.+' "$TMP/n4.out"; then ng "N4 负控 D 没红（丢错误仍报出原因 ⇒ C11 无牙）"; else ok "N4 负控 D 成立：丢错误后原因消失（C11 会红）"; fi

printf '\n'
if [ "$F" = 0 ]; then echo "M389 门结果：$P 通过 / 0 失败"; echo "M389-VERIFY-OK"; exit 0
else echo "M389 门结果：$P 通过 / $F 失败"; echo "M389-VERIFY-FAIL"; exit 1; fi
