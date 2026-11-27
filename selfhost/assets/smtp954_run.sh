#!/bin/bash
# ============================================================
# smtp954_run.sh —— 缺陷 954（M448）E1 判据执行体
#   配对：smtp954_server.py（假 SMTP 端·三模式）+ smtp954_client.px（std.smtp 客户端）
#   用法：smtp954_run.sh [仓库根]   （默认取本脚本上两级）
#   退出：0 全绿 / 1 有红（每条断言都会打印 want/got）
#   端口：服务端 --port 0 由内核分配 ⇒ 可并发、不撞端口
# ============================================================
set -u
D=$(cd "$(dirname "$0")" && pwd)
REPO=${1:-$(cd "$D/../.." && pwd)}
OUT=${SMTP954_OUT:-/data/gatevm/m448-954/out}
PY=python3
pass=0; fail=0
ck() { if [ "$2" = "$3" ]; then echo "✅ $1"; pass=$((pass+1)); else echo "❌ $1（want [$3] got [$2]）"; fail=$((fail+1)); fi; }
jget() { $PY -c "import json,sys;d=json.load(open(sys.argv[1]));print(d.get(sys.argv[2]))" "$1" "$2" 2>/dev/null; }

rm -rf "$OUT"; mkdir -p "$OUT"
cd "$D" || exit 1
# 自签名证书（只在回环，verify=false）
openssl req -x509 -newkey rsa:2048 -nodes -keyout "$OUT/key.pem" -out "$OUT/cert.pem" \
    -days 2 -subj "/CN=localhost" >/dev/null 2>&1 || { echo "❌ 证书生成失败"; exit 1; }

# 客户端：px build 把产物固定写在源文件旁的 build/ ⇒ 先拷到临时目录再编译，避免污染仓库
SRC=$OUT/src; mkdir -p "$SRC"
cp "$D/smtp954_client.px" "$SRC/"
( cd "$REPO" && ./tools/px build "$SRC/smtp954_client.px" ) > "$OUT/build.log" 2>&1 \
    || { echo "❌ 客户端编译失败"; tail -5 "$OUT/build.log"; exit 1; }
BIN=$SRC/build/smtp954_client
ck "前提：客户端编译成功" "$([ -x "$BIN" ] && echo yes || echo no)" yes

# run <tag> <server-mode> <client-mode> [advertise]
run() {
    local tag=$1 smode=$2 cmode=$3 adv=${4:-}
    local rj="$OUT/$tag.json" rl="$OUT/$tag.ready" out="$OUT/$tag.client.txt"
    $PY "$D/smtp954_server.py" --port 0 --mode "$smode" \
        --cert "$OUT/cert.pem" --key "$OUT/key.pem" --out "$rj" $adv > "$rl" 2>&1 &
    local spid=$!
    local port=""
    for i in $(seq 1 40); do
        port=$($PY -c "import sys;l=open(sys.argv[1]).readline().split();print(l[1] if len(l)>1 and l[0]=='READY' else '')" "$rl" 2>/dev/null)
        [ -n "$port" ] && break; sleep 0.15
    done
    if [ -z "$port" ]; then echo "❌ 前提：[$tag] 假端未就绪：$(head -2 "$rl")"; fail=$((fail+1)); return 1; fi
    timeout 90 "$BIN" 127.0.0.1 "$port" "$cmode" > "$out" 2>&1
    wait $spid 2>/dev/null
    echo "  [$tag] $(tail -1 "$out")"
    return 0
}

# ── S1 默认明文：不传 opts，行为与 M83 一致 ──
run s1 plain plain
ck "S1 明文默认 ok=true" "$(grep -c 'ok=true' "$OUT/s1.client.txt")" 1
ck "S1b 服务端记录 tls=false（前提自证）" "$(jget "$OUT/s1.json" tls)" False
ck "S1c 消息体真的送达（data_bytes>200）" "$([ "$(jget "$OUT/s1.json" data_bytes)" -gt 200 ] && echo yes || echo no)" yes
ck "S1d Subject 原样送达" "$(jget "$OUT/s1.json" subject)" "M448-954 STARTTLS test"

# ── S2 STARTTLS：广告→升级→重新 EHLO→发信 ──
run s2 starttls starttls
ck "S2 STARTTLS ok=true" "$(grep -c 'ok=true' "$OUT/s2.client.txt")" 1
ck "S2b 连接确实升级为 TLS" "$(jget "$OUT/s2.json" tls)" True
ck "S2c 升级后重新 EHLO（RFC 3207）" "$(jget "$OUT/s2.json" ehlo_after_tls)" True
ck "S2d 走的是现代 TLS 版本" "$(case "$(jget "$OUT/s2.json" tls_version)" in TLSv1.2|TLSv1.3) echo yes;; *) echo no;; esac)" yes
ck "S2e 消息体在 TLS 内送达（data_bytes 与明文一致 262）" "$(jget "$OUT/s2.json" data_bytes)" "$(jget "$OUT/s1.json" data_bytes)"

# ── S3 implicit TLS（465 风格）──
run s3 implicit implicit
ck "S3 implicit ok=true" "$(grep -c 'ok=true' "$OUT/s3.client.txt")" 1
ck "S3b 全程 TLS（tls=true）" "$(jget "$OUT/s3.json" tls)" True

# ── S4 负控 A：广告了 STARTTLS 但服务端拒绝 ⇒ 必须报错、不许退明文发信 ──
run s4 advertise-fake starttls --advertise-starttls
ck "S4 升级失败 ⇒ ok=false" "$(grep -c 'ok=false' "$OUT/s4.client.txt")" 1
ck "S4b 服务端没收到明文 MAIL FROM（真没降级）" "$(jget "$OUT/s4.json" data_bytes)" 0

# ── S5 负控 B：服务端不广告 STARTTLS ⇒ 报错点名「未通告」──
run s5 plain starttls
ck "S5 未通告 ⇒ ok=false" "$(grep -c 'ok=false' "$OUT/s5.client.txt")" 1
ck "S5b 报错点名「未通告」" "$(grep -c '未通告' "$OUT/s5.client.txt")" 1

# ── S6 非法值：一律响亮报错 ──
run s6 plain bogus
ck "S6 非法 tls 值 ⇒ ok=false" "$(grep -c 'ok=false' "$OUT/s6.client.txt")" 1
ck "S6b 报错点名可接受值" "$(grep -c '只接受' "$OUT/s6.client.txt")" 1

echo "----------------------------------------"
echo "M448_SMTP_TLS pass=$pass fail=$fail"
[ "$fail" = 0 ] || exit 1
