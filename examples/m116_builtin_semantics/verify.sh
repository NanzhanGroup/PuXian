#!/usr/bin/env bash
# ============================================================
# M116 验证（qg-issue 71 D2/D4/D5/D6/D7）
# ------------------------------------------------------------
# 判据（每条都直接比可观测事实，不靠"应该没问题"）：
#   ① D4/D6 + D2 守卫写法：双轨（px run / px build）都必须全 PASS
#   ② D2 负控：**两条轨**跑 dict_missing_key.px 都必须非零退出、stderr 含 R1008
#      且出错点之后的 "after" 不得出现（两轨同口径 = 修复的核心判据）
#   ③ D6：mkdir mode 按 umask 折算的绝对权限位断言（在 ① 内）
#   ④ D5：dns_txt TCP 回退（多记录域名）—— 需外网，离线记 SKIP 不计红
#   ⑤ D7 负控：handler 出错 → **500** + **服务存活**（VM 轨与 C 轨各跑一遍）
#   ⑥ 门自检负控：把断言值改错必须变红（防门自欺）
# 用法：bash examples/m116_builtin_semantics/verify.sh
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m116_builtin_semantics
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"; [ -n "${SRV_PID:-}" ] && kill "$SRV_PID" 2>/dev/null' EXIT

pass=0; fail=0; skip=0
ok()   { echo "  ✅ $1"; pass=$((pass+1)); }
bad()  { echo "  ❌ $1"; fail=$((fail+1)); }
skips(){ echo "  ⏭  $1"; skip=$((skip+1)); }

echo "== M116 验证（PuXian 缺陷集 D2/D4/D5/D6/D7）=="

# ---------- ① D4/D6/D2 守卫：双轨 ----------
echo "[1/6] 双轨语义断言（$DIR/m116_semantics.px）"
./tools/px run "$DIR/m116_semantics.px" > "$TMP/i.log" 2>&1; rc_i=$?
if [ $rc_i -eq 0 ] && tail -1 "$TMP/i.log" | grep -q 'M116_SEMANTICS:.*0F'; then
    ok "解释轨（px run）：$(tail -1 "$TMP/i.log")"
else
    bad "解释轨失败 rc=$rc_i"; tail -12 "$TMP/i.log"
fi
./tools/px build "$DIR/m116_semantics.px" > "$TMP/b.log" 2>&1
if [ -x "$DIR/build/m116_semantics" ] &&
   ./"$DIR/build/m116_semantics" > "$TMP/c.log" 2>&1 &&
   tail -1 "$TMP/c.log" | grep -q 'M116_SEMANTICS:.*0F'; then
    ok "编译轨（px build，VM 默认轨）：$(tail -1 "$TMP/c.log")"
else
    bad "编译轨失败"; tail -12 "$TMP/c.log" 2>/dev/null; tail -5 "$TMP/b.log"
fi

# ---------- ② D2 负控：两轨同口径 ----------
echo "[2/6] D2 负控：缺键下标两轨都必须以 R1008 失败"
d2_probe() {   # $1=标签 $2...=命令
    local label="$1"; shift
    local out="$TMP/d2.$label.log"
    "$@" > "$out" 2>&1; local rc=$?
    if [ $rc -eq 0 ]; then bad "$label：缺键竟成功退出（静默 null 回归）"; return; fi
    if ! grep -q 'R1008' "$out"; then bad "$label：退出码非零但无 R1008（判据不符）"; sed -n 1,3p "$out"; return; fi
    if ! grep -q "字典没有键 'nope'" "$out"; then bad "$label：R1008 文案缺键名"; sed -n 1,3p "$out"; return; fi
    if grep -q '^after$' "$out"; then bad "$label：出错后仍继续执行（不是硬错误）"; return; fi
    ok "$label：rc=$rc · $(grep -m1 R1008 "$out" | sed 's/^ *//')"
}
d2_probe "解释轨" ./tools/px run "$DIR/dict_missing_key.px"
./tools/px build "$DIR/dict_missing_key.px" > "$TMP/d2.build.log" 2>&1
d2_probe "编译轨(VM)" ./"$DIR/build/dict_missing_key"
./tools/pxc build "$DIR/dict_missing_key.px" > "$TMP/d2.buildc.log" 2>&1
if [ -x "$DIR/build/dict_missing_key" ]; then
    cp -f "$DIR/build/dict_missing_key" "$TMP/d2c.bin" 2>/dev/null
fi
# C 轨另编一份（tools/pxc 与 tools/px 输出同名同目录，故用 PX_BUILD_ENGINE 隔离）
PX_BUILD_ENGINE=c ./tools/px build "$DIR/dict_missing_key.px" > "$TMP/d2.buildc2.log" 2>&1
if [ -x "$DIR/build/dict_missing_key" ]; then
    cp -f "$DIR/build/dict_missing_key" "$TMP/d2c.bin"
    d2_probe "编译轨(C)" "$TMP/d2c.bin"
fi

# ---------- ③ D5：dns_txt TCP 回退（需外网） ----------
echo "[3/6] D5：dns_txt UDP 截断 → TCP 回退（多记录域名）"
cat > "$TMP/d5.px" <<'PXEOF'
# 先探网：getaddrinfo 路径（dns_lookup）失败 ⇒ 环境无外网 ⇒ 退出码 2（门记 SKIP）
let probe = dns_lookup("google.com")
if type(probe) == "result":
    print("NET-DOWN: " + str(probe))
    exit(2)
let names = ["google.com", "cloudflare.com"]
var bad = 0
for n in names:
    let r = dns_txt(n)
    if type(r) == "result":
        print("ERR " + n + ": " + str(r))
        bad += 1
    else:
        print("OK " + n + " n=" + str(len(r)))
if bad > 0:
    exit(1)
PXEOF
./tools/px build "$TMP/d5.px" > "$TMP/d5.build.log" 2>&1
"$TMP/build/d5" > "$TMP/d5.log" 2>&1; d5rc=$?
if [ $d5rc -eq 0 ]; then
    ok "TCP 回退生效：$(tr '\n' ' ' < "$TMP/d5.log")"
elif [ $d5rc -eq 2 ]; then
    skips "环境无外网（dns_lookup 已失败），跳过：$(tr '\n' ' ' < "$TMP/d5.log")"
else
    bad "dns_txt 多记录域名失败（网通则必红）：$(tr '\n' ' ' < "$TMP/d5.log")"
fi

# ---------- ④ D7：handler 出错 → 500 + 服务存活 ----------
echo "[4/6] D7 负控：handler 运行时错误 → 500（且服务不死）"
if ! command -v curl >/dev/null 2>&1; then
    skips "无 curl，跳过 D7 端到端"
else
    mkdir -p /tmp/m116_docroot
    d7_case() {   # $1=标签 $2=产物路径
        local label="$1" bin="$2"
        local port=$((20000 + RANDOM % 20000))
        M116_PORT=$port "$bin" > "$TMP/d7.$label.log" 2>&1 &
        local pid=$!
        sleep 1.5
        local code body
        code=$(curl -s -o "$TMP/d7.body" -w '%{http_code}' --max-time 5 "http://127.0.0.1:$port/bad" 2>/dev/null || echo 000)
        body=$(head -c 60 "$TMP/d7.body" 2>/dev/null | tr -d '\n')
        local alive
        alive=$(curl -s -o /dev/null -w '%{http_code}' --max-time 5 "http://127.0.0.1:$port/nonexist" 2>/dev/null || echo 000)
        kill $pid 2>/dev/null; wait $pid 2>/dev/null
        if [ "$code" != "500" ]; then bad "$label：出错 handler 返回 $code（应为 500）"; return; fi
        if [ "$alive" = "000" ]; then bad "$label：服务在 handler 出错后死亡（应继续可用）"; return; fi
        ok "$label：500 + 服务存活（后续请求 $alive）· body=\"$body\""
    }
    ./tools/px build "$DIR/serve_err.px" > "$TMP/d7.build.log" 2>&1
    if [ -x "$DIR/build/serve_err" ]; then
        d7_case "VM轨" "$DIR/build/serve_err"
    else
        bad "VM 轨 serve_err 编译失败"; tail -5 "$TMP/d7.build.log"
    fi
    PX_BUILD_ENGINE=c ./tools/px build "$DIR/serve_err.px" > "$TMP/d7.buildc.log" 2>&1
    if [ -x "$DIR/build/serve_err" ]; then
        cp -f "$DIR/build/serve_err" "$TMP/serve_err_c"
        d7_case "C轨" "$TMP/serve_err_c"
    else
        bad "C 轨 serve_err 编译失败"; tail -5 "$TMP/d7.buildc.log"
    fi
fi

# ---------- ⑤ 门自检负控 ----------
echo "[5/6] 负控：断言值改错必须变红"
sed 's/== \["xy", "z"\]/== ["xy", "NOPE"]/' "$DIR/m116_semantics.px" > "$TMP/neg.px"
if ./tools/px run "$TMP/neg.px" > "$TMP/neg.log" 2>&1; then
    bad "负控未生效：故意错值仍然通过（门在自欺）"
else
    ok "负控生效（故意错值 → 红）"
fi

# ---------- ⑥ 反例自检：D4 修复前的"函数体被截断"必须不复现 ----------
echo "[6/6] D4 反例自检：多行字面量之后的本函数语句仍属本函数"
cat > "$TMP/d4tail.px" <<'PXEOF'
def f():
    var xs = [
        1,
        2
    ]
    xs.append(3)
    return xs
def g():
    return {"k": 1,
            "j": 2}
let a = f()
let b = g()
print("A=" + str(a) + " B=" + str(b["j"]))
PXEOF
./tools/px run "$TMP/d4tail.px" > "$TMP/d4tail.log" 2>&1
if grep -q 'A=\[1, 2, 3\] B=2' "$TMP/d4tail.log"; then
    ok "续行后函数体完整：$(tail -1 "$TMP/d4tail.log")"
else
    bad "续行破坏函数体（D4 回归）"; tail -5 "$TMP/d4tail.log"
fi

# ---------- 清理 ----------
rm -rf "$DIR/build" /tmp/build/d5 "$DIR/../build/m116_semantics" 2>/dev/null

echo ""
echo "M116_VERIFY: ${pass}P/${fail}F/${skip}S"
[ "$fail" -eq 0 ] || exit 1
exit 0
