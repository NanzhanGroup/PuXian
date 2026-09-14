# m118_realworld_defects · 三处「写不出来 / 写错才报」的缺陷 + 一处流式客户端缺口
#
# 探针来源：NanzhanGroup/chat（文殊聊天引擎，Go 8793 行）整体 PuXian 化。
# 本目录的用例全部来自**移植时的真实现场**，不是造出来的。
#   1. 插值里带字符串字面量：`print("v=${d[\"k\"]}")` → 旧版 E1001 非法字符 '\'（两轨）
#   2. 匿名函数多行体：`var f = fn ():` / 调用实参里 `map(xs, fn (x):` → 旧版 E2001 意外换行
#   3. `spawn <闭包>`：旧版把编译器内部函数名 + AST dump 抛给用户（无源位置、无建议）
#   4. `sse_connect` 只支持 GET/TCP：LLM 走 Unix socket 的 POST+SSE 无法表达（chat 的核心通路）
#
# 用法：bash examples/m118_realworld_defects/verify.sh
set -u
cd "$(dirname "$0")/../.."
PX=./tools/px
TMP=$(mktemp -d /tmp/m118_XXXXXX)
trap 'rm -rf "$TMP"' EXIT
FAIL=0
ok()   { echo "  ✅ $1"; }
bad()  { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

echo "── 1. 插值内字符串字面量（\${d[\"k\"]} / \${f(s, \"x\")}）"
cat > "$TMP/t1.px" <<'PXEOF'
def main():
    var d = json_parse("{\"k\":\"v\"}")
    var s = "ab"
    print("k=${d[\"k\"]} len=${len(s)} c=${contains(s, \"a\")} j=${join(\",\", [\"x\", \"y\"])}")
PXEOF
if out=$($PX run "$TMP/t1.px" 2>&1) && [ "$out" = "k=v len=2 c=true j=x,y" ]; then ok "解释轨"; else bad "解释轨: $out"; fi
if $PX build "$TMP/t1.px" >/dev/null 2>&1 && out=$("$TMP/build/t1" 2>&1) && [ "$out" = "k=v len=2 c=true j=x,y" ]; then ok "编译轨"; else bad "编译轨: $out"; fi

echo "── 2. 匿名函数多行体（语句位 + 调用实参位）"
cat > "$TMP/t2.px" <<'PXEOF'
def main():
    var f = fn (x):
        var t = x * 3
        return t
    print("f=", f(2))
    var ys = [1, 2, 3, 4]
    var evens = filter(ys, fn (x):
        var r = x % 2
        return r == 0)
    print("evens=", json_stringify(evens))
PXEOF
if out=$($PX run "$TMP/t2.px" 2>&1 | tr '\n' '|') && [ "$out" = "f= 6|evens= [2,4]|" ]; then ok "解释轨"; else bad "解释轨: $out"; fi
if $PX build "$TMP/t2.px" >/dev/null 2>&1 && out=$("$TMP/build/t2" 2>&1 | tr '\n' '|') && [ "$out" = "f= 6|evens= [2,4]|" ]; then ok "编译轨（VM 轨）"; else bad "编译轨: $out"; fi

echo "── 3. spawn <闭包>：必须是 E2011 可读诊断（不再泄漏内部 AST）"
cat > "$TMP/t3.px" <<'PXEOF'
def main():
    spawn fn (): print("hi")
    sleep(0.1)
PXEOF
err=$($PX build "$TMP/t3.px" 2>&1 | tr '\n' ' ')
if echo "$err" | grep -q "E2011" && echo "$err" | grep -q "spawn f(args)"; then ok "诊断含 E2011 + 可用写法"; else bad "诊断: $err"; fi
if echo "$err" | grep -q "Closure\|bc_emit"; then bad "仍泄漏内部实现细节: $err"; else ok "未泄漏内部函数名/AST"; fi

echo "── 4. sse_connect：Unix socket + POST + 自定义头（一次性 SSE 响应体）"
cat > "$TMP/t4.px" <<'PXEOF'
def handler(req):
    var b = "data: {\"i\":1}\n\ndata: {\"i\":2}\n\ndata: [DONE]\n\n"
    return {"status": 200, "body": b, "headers": {"Content-Type": "text/event-stream"}}
def main():
    var sock = "/tmp/m118_sse.sock"
    spawn http_serve_unix(sock, handler)
    sleep(0.6)
    var opts = json_parse("{}")
    opts.set("sock", sock)
    opts.set("method", "POST")
    opts.set("body", "{\"stream\":true}")
    var hdrs = json_parse("{}")
    hdrs.set("Authorization", "Bearer t")
    opts.set("headers", hdrs)
    var id = sse_connect("/v1/chat/completions", opts)
    if id == null:
        print("FAIL connect")
        exit(1)
    var n = 0
    var last = ""
    while true:
        var ev = sse_read(id)
        if ev == null:
            break
        n = n + 1
        last = ev["data"]
        if contains(last, "[DONE]"):
            break
    sse_close(id)
    print("events=", n, " last=", last)
PXEOF
if $PX build "$TMP/t4.px" >/dev/null 2>&1 && out=$("$TMP/build/t4" 2>&1) && [ "$out" = "events= 3  last= [DONE]" ]; then ok "unix+POST+自定义头+SSE 解析通（3 事件）"; else bad "unix+POST+SSE: $out"; fi

echo "── 5. 负控：坏 socket / 非流式响应必须优雅失败（不 panic、不挂死）"
cat > "$TMP/t5.px" <<'PXEOF'
def main():
    var opts = json_parse("{}")
    opts.set("sock", "/tmp/m118_not_exist.sock")
    opts.set("method", "POST")
    opts.set("body", "{}")
    var id = sse_connect("/x", opts)
    print("bad_sock_id=", id)
    var id2 = sse_connect("unix:///tmp/x.sock/v1")
    print("old_form_id=", id2)
PXEOF
if $PX build "$TMP/t5.px" >/dev/null 2>&1 && out=$("$TMP/build/t5" 2>&1 | tr '\n' '|') && [ "$out" = "bad_sock_id= null|old_form_id= null|" ]; then ok "坏 socket → null，旧 URL 形式 → null（零回归）"; else bad "负控: $out"; fi

echo "── 6. 负控：M117 的 POSIX 类名仍要报错（防本轮改动回退）"
cat > "$TMP/t6.px" <<'PXEOF'
def main():
    var r = regex_search("[[:nope:]]", "x")
    print("should_not_reach=", r)
PXEOF
err=$($PX run "$TMP/t6.px" 2>&1 | tr '\n' ' ')
if echo "$err" | grep -q "字符类"; then ok "未知 POSIX 类名 → 运行期明确报错"; else bad "负控: $err"; fi

echo
if [ "$FAIL" = "0" ]; then echo "✅ m118 全部用例通过"; else echo "❌ 失败 $FAIL 项"; fi
exit $FAIL
