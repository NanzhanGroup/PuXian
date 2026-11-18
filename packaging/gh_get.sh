#!/usr/bin/env bash
# ============================================================
# packaging/gh_get.sh —— GitHub API 取数的**唯一实现**（M389 · 缺陷 709）
#
# 为什么有它（2026-11-10 实测事故链）：
#   /data/pat.md 的令牌在 15:43 之后失效。旧写法是
#       curl -sS -H "Authorization: Bearer $PAT" ... | grep -c '"browser_download_url"'
#   401 的响应体是 {"message":"Bad credentials"} ⇒ grep 得 0 ⇒ **assets=0**
#   ⇒ 判据把「测量失败」当成「测量结果：资产还没出」。
#   同族已发生的五例：696（全绿判 RED）· 697（双发布）· 703（抢锁 exit 0）
#   705（预警档死）· 707（假报成功）。**共同点：不崩溃，只是安静地少算或错报。**
#
# 本文件提供的契约（调用方只许用这三个）：
#   gh_get <path> <outfile>
#       0  取到可解释的答案（200 有效 JSON / 404 显式不存在）
#       3  令牌失效，已降级匿名取到答案（答案可用，但**必须在日志里留痕**）
#       4  取不到数 —— 调用方必须**响亮失败**，绝不允许当成「空结果」
#   副作用：GH_LAST_CODE（最后一次 HTTP 码）· GH_AUTH_STATE
#           （token-ok / token-invalid-anon-ok / anon-ok / fail-<code>）
#   gh_auth_state        打印一行人类可读状态（写日志用，不含令牌本体）
#
# 环境：
#   GH_TOKEN      优先；缺省从 GH_PAT_FILE（默认 /data/pat.md）读，权限 600
#   GH_API        默认 https://api.github.com
#   GH_ALLOW_ANON 默认 1；置 0 表示禁止匿名降级（需要写权限时用，例如 PATCH）
#   GH_MAX_TIME   curl 超时秒数（默认 20）
#
# [SECURITY/P0] 令牌只走环境变量；本文件**任何路径都不打印令牌本体**，
#              日志里只出现长度与类型前缀（gh_auth_state 负责）。
# ============================================================
set -uo pipefail

GH_API="${GH_API:-https://api.github.com}"
GH_PAT_FILE="${GH_PAT_FILE:-/data/pat.md}"
GH_ALLOW_ANON="${GH_ALLOW_ANON:-1}"
GH_MAX_TIME="${GH_MAX_TIME:-20}"
# M415 · 缺陷 839（2026-11-18 实测）：传输层抖动不重试 ⇒ 一次 20s 超时就判 ENV-FAIL。
#   实测 8 次取数里 1 次卡满 20s（到 api.github.com connect 0.3s / TLS 0.6s，路径本就慢），
#   而 gh_get 对 code=000 **一次就返回 4** ⇒ dy-ops-ci-parity 21 分钟前刚绿、这一轮就 p1 叫醒。
#   「测量失败要响亮」不等于「不许重试」：重试只针对**没有答案**的情形（传输失败 / 限流 / 5xx），
#   语义答案（401/403/404）一次就够，绝不重试。重试完仍失败 ⇒ 依旧 rc=4（响亮不变）。
GH_RETRY="${GH_RETRY:-3}"              # 最多尝试次数（含首次）
GH_RETRY_SLEEP="${GH_RETRY_SLEEP:-2}"  # 退避基数：第 n 次失败后睡 n*base 秒
GH_QUIET_FAIL="${GH_QUIET_FAIL:-0}"    # 1 = 不把「取不到数」的说明写到 stderr

GH_LAST_CODE=""
GH_LAST_ATTEMPTS=0
GH_AUTH_STATE="unset"

# ── 令牌装载：去掉所有空白（文件天然带换行，带进 header 会畸形 → 401）──
_gh_token() {
    if [ -n "${GH_TOKEN:-}" ]; then printf '%s' "$(printf '%s' "$GH_TOKEN" | tr -d ' \t\r\n')"; return 0; fi
    [ -r "$GH_PAT_FILE" ] || return 1
    local t; t="$(tr -d ' \t\r\n' < "$GH_PAT_FILE" 2>/dev/null)"
    [ -n "$t" ] || return 1
    printf '%s' "$t"
}

# ── 单次请求：$1=path $2=outfile $3=token(可空) ⇒ 吐 http code（curl 失败吐 000）──
# ⚠️ M415 · 缺陷 831：**令牌不许进 argv**（2026-11-18 实测：systemd 的 cgroup 树把
#   `curl ... -H "Authorization: Bearer github_pat_..."` 整条打进 journal ⇒ 任何能
#   读日志/ps 的本机进程都能拿到有效期 30 天的 PAT）。改用 `curl -K -`（配置从 stdin 读），
#   argv 里只剩 `-K -`。gh_auth_state 本来就只报长度/前缀，这条把「本体不落地」补齐。
_gh_authhdr_cfg() {  # $1=token ⇒ 一行 curl 配置文件（不进 argv）
    printf 'header = "Authorization: Bearer %s"\n' "$1"
}
_gh_call() {
    local path="$1" out="$2" tok="${3:-}" code
    if [ -n "$tok" ]; then
        code="$( _gh_authhdr_cfg "$tok" | curl -sS -o "$out" -w '%{http_code}' --max-time "$GH_MAX_TIME" \
                     -K - -H "Accept: application/vnd.github+json" "$GH_API$path" 2>/dev/null )" || code="000"
    else
        code="$( curl -sS -o "$out" -w '%{http_code}' --max-time "$GH_MAX_TIME" \
                     -H "Accept: application/vnd.github+json" "$GH_API$path" 2>/dev/null )" || code="000"
    fi
    printf '%s' "${code:-000}"
}

# ── 传输层失败是否值得重试：只重试「没有答案」，不重试「答案就是不许」 ──
#   000 = 连不上/超时/被截断 · 429 = 限流 · 5xx = 服务端出错；这三种换个时间问可能有答案。
#   401/403/404 = 令牌/权限/资源，问一百次也是同一个答案，重试只是浪费且掩盖真因。
_gh_retryable() { case "$1" in 000|429|500|502|503|504) return 0 ;; *) return 1 ;; esac }

# ── 带退避重试的取数：结果写 GH_LAST_CODE，尝试次数写 GH_LAST_ATTEMPTS ──
#   ⚠️ 结果走全局变量而不是 stdout：`code="$(...)"; 在子 shell 里，GH_LAST_ATTEMPTS 会丢
_gh_call_retry() {
    local path="$1" out="$2" tok="${3:-}" code="" n=1
    while :; do
        code="$(_gh_call "$path" "$out" "$tok")"
        if _gh_retryable "$code" && [ "$n" -lt "$GH_RETRY" ]; then
            [ "${GH_DEBUG:-0}" = "1" ] && printf 'gh_get: 第 %s/%s 次失败 code=%s ⇒ 退避重试\n' "$n" "$GH_RETRY" "$code" >&2
            sleep $(( n * GH_RETRY_SLEEP ))
            n=$((n + 1))
            continue
        fi
        break
    done
    GH_LAST_CODE="$code"
    GH_LAST_ATTEMPTS="$n"
    if _gh_retryable "$code" && [ "$GH_QUIET_FAIL" = "0" ]; then
        printf 'gh_get: 取不到数 code=%s attempts=%s（重试已用完；测量失败 ≠ 空结果）\n' "$code" "$n" >&2
    fi
}

# ── 响应体是否「可解释」（{...} 或 [...]）：防止把 HTML 错误页当 JSON 用 ──
_gh_json_ok() {
    local f="$1" first
    [ -s "$f" ] || return 1
    first="$(head -c 1 "$f" 2>/dev/null)"
    [ "$first" = "{" ] || [ "$first" = "[" ]
}

# ── 主入口 ────────────────────────────────────────────────────
gh_get() {
    local path="$1" out="$2" tok code anon_code
    tok="$(_gh_token || true)"

    if [ -n "$tok" ]; then
        _gh_call_retry "$path" "$out" "$tok"; code="$GH_LAST_CODE"
        GH_LAST_CODE="$code"
        case "$code" in
            200)
                if _gh_json_ok "$out"; then GH_AUTH_STATE="token-ok"; return 0; fi
                # 200 但不是 JSON ⇒ 中间设备插页/HTML，按「取不到数」处理，不许当空结果
                GH_AUTH_STATE="fail-200-notjson"; return 4 ;;
            401|403)
                GH_AUTH_STATE="token-invalid" ;;      # 落到下面决定是否匿名降级
            404)
                GH_AUTH_STATE="notfound"; return 0 ;;  # 404 = 资源不存在，是合法答案
            *)
                GH_AUTH_STATE="fail-$code" ; return 4 ;;
        esac
        # 令牌失效：能否匿名？
        [ "$GH_ALLOW_ANON" = "1" ] || { GH_AUTH_STATE="token-invalid-no-anon"; return 4; }
    else
        # 根本没有令牌：允许匿名才继续，否则按「取不到数」响亮失败（不许静默空返回）
        [ "$GH_ALLOW_ANON" = "1" ] || { GH_AUTH_STATE="no-token-no-anon"; return 4; }
    fi

    _gh_call_retry "$path" "$out" ""; anon_code="$GH_LAST_CODE"
    GH_LAST_CODE="$anon_code"
    case "$anon_code" in
        200)
            if _gh_json_ok "$out"; then
                if [ -n "$tok" ]; then GH_AUTH_STATE="token-invalid-anon-ok"; return 3; fi
                GH_AUTH_STATE="anon-ok"; return 0
            fi
            GH_AUTH_STATE="fail-200-notjson"; return 4 ;;
        404) GH_AUTH_STATE="notfound"; return 0 ;;
        *)   GH_AUTH_STATE="fail-$anon_code"; return 4 ;;
    esac
}

# ── 需要写权限的调用（PATCH 等）：禁止匿名降级，且必须显式说明失败原因 ──
gh_write() {  # gh_write <method> <path> <body> <outfile>
    local method="$1" path="$2" body="$3" out="$4" tok code
    tok="$(_gh_token || true)"
    [ -n "$tok" ] || { GH_AUTH_STATE="token-missing"; return 4; }
    code="$( _gh_authhdr_cfg "$tok" | curl -sS -o "$out" -w '%{http_code}' --max-time "$GH_MAX_TIME" -X "$method" \
                 -K - -H "Accept: application/vnd.github+json" \
                 -d "$body" "$GH_API$path" 2>/dev/null )" || code="000"
    GH_LAST_CODE="$code"
    case "$code" in
        200|201|202|204) GH_AUTH_STATE="write-ok"; return 0 ;;
        401|403)     GH_AUTH_STATE="write-token-invalid"; return 4 ;;
        *)           GH_AUTH_STATE="write-fail-$code"; return 4 ;;
    esac
}

# ── 人类可读状态（**不含令牌本体**，只报有无/长度/前缀类型）──
gh_auth_state() {
    local tok len=0 kind=none
    tok="$(_gh_token || true)"
    if [ -n "$tok" ]; then
        len=${#tok}
        case "$tok" in
            github_pat_*) kind=fine-grained ;;
            ghp_*)        kind=classic ;;
            *)            kind=other ;;
        esac
    fi
    printf 'auth=%s source=%s len=%s kind=%s patfile=%s allow_anon=%s\n' \
        "$GH_AUTH_STATE" "$([ -n "${GH_TOKEN:-}" ] && echo env || echo file)" \
        "$len" "$kind" "$GH_PAT_FILE" "$GH_ALLOW_ANON"
}
