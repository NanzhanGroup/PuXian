#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M384 · PuXian 运行时/词法缺陷收口（清歌清单 34 条 → 只做「纯增益 5 条」）
# ---------------------------------------------------------------------------
# 口径（用户 2026-11-09 22:52 令「动 runtime/*.c」）：只做**不改既有可观测行为**的 5 条；
#   会改语义的 3 条、能力缺口 20 条一律留在队列（NEXT-QUEUE.md）。
#
# 五条缺陷（清歌编号 → 本轮缺陷号）：
#   PX-001 缺陷 683  xml_unescape("") 段错误（rc=139）—— XML 空元素极常见，wst-s3 首跑即踩
#   PX-005 缺陷 685  base64 宽松到 "QQ"/"QQ=" 都解出 "A"（Go 判无效）⇒ 验签可被绕过
#   PX-011 缺陷 686  json_stringify_go 缺 \b \f（Go **实测**给 "x\by\fz"，不是 \u0008）
#   PX-014 缺陷 689  词法不支持 \b \f \v \xNN（一律 E1005）
#   PX-024 缺陷 688  write_bytes("/dev/stdout") 走 O_TRUNC ⇒ 重定向到普通文件只留最后一段
#
# 判据（每条都配能红的负控）：
#   [1] 空串不崩（rc=0 且返回空串）· [1s] 静态：detach 有 NULL 兜底 + unescape 不裸 px_str
#       [1s 负控] 从副本删掉 guard ⇒ 扫描器必须报警
#   [2] 严格变体 10 条规则 · [2 负控]（内嵌在 r[10]/r[11]）宽松版必须原样仍是 "A"
#   [3] \b \f 逐字节等于 Go；控制字符仍 \u00XX · [3 负控] json_stringify 仍输出裸字节
#   [4] \b \f \v \xNN 可编且语义对 · [4 负控] "\q" 必须仍是 E1005
#   [5] 两条 /dev/stdout 写入都留 · [5 负控] 普通路径写两次仍只留最后一次（O_TRUNC 不许被改）
#       · [5s] 静态：bi_write_bytes 内有 fd 直写分支
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
TMP="${TMPDIR:-/tmp}/m384_defects.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

# ── [1] PX-001 空输入不崩 ───────────────────────────────────────────────
cat > "$TMP/p1.px" <<'PXEOF'
print("A" + xml_unescape("") + "B")
print("len=" + str(len(xml_unescape(""))))
print(xml_unescape("&lt;x&amp;y&gt;"))
PXEOF
out=$("$PX" run "$TMP/p1.px" 2>"$TMP/p1.err"); rc=$?
if [ "$rc" = 0 ]; then
    ok "[1] xml_unescape(\"\") 不崩（rc=0；修前 rc=139 SIGSEGV）"
else
    bad "[1] xml_unescape(\"\") 仍崩：rc=$rc（$(head -1 "$TMP/p1.err")）"
fi
[ "$(echo "$out" | sed -n '1p')" = "AB" ] && ok "[1] 空串 → 空串（不是 null）" \
    || bad "[1] 空串结果不对：$(echo "$out" | sed -n '1p')"
[ "$(echo "$out" | sed -n '2p')" = "len=0" ] && ok "[1] len(xml_unescape(\"\")) == 0" \
    || bad "[1] len 不对：$(echo "$out" | sed -n '2p')"
[ "$(echo "$out" | sed -n '3p')" = "<x&y>" ] && ok "[1] 非空路径未受影响（实体解码照旧）" \
    || bad "[1] 实体解码回归：$(echo "$out" | sed -n '3p')"

# [1s] 静态：NULL 兜底在场（**剥注释后**扫 —— 注释里的代码不算实现）
detach_body=$(awk '/static char\* xbuf_detach/,/^}/' runtime/runtime_xml.c | grep -v '^[[:space:]]*//')
echo "$detach_body" | grep -q 'if (!d)' \
    && ok "[1s] xbuf_detach 有 NULL 兜底分支" \
    || bad "[1s] xbuf_detach 的 NULL 兜底不见了（空输入会重新返回 NULL）"
unesc_body=$(awk '/bi_xml_unescape/,/^}/' runtime/runtime_xml.c | grep -v '^[[:space:]]*//')
echo "$unesc_body" | grep -q 'dec ? px_str(dec) : px_str("")' \
    && ok "[1s] bi_xml_unescape 有第二道保险（不裸 px_str(NULL)）" \
    || bad "[1s] bi_xml_unescape 又直接 px_str(dec) 了"
# 负控：从**副本**里抹掉 guard，扫描器必须报警（证明 [1s] 不是恒真）
awk '{
    if ($0 ~ /if \(!d\)/) { print "    /* guard removed by negative control */"; next }
    print
}' runtime/runtime_xml.c > "$TMP/xml_tamper.c"
if grep -q 'if (!d)' <(awk '/static char\* xbuf_detach/,/^}/' "$TMP/xml_tamper.c"); then
    bad "[1s] 负控失败：删掉 guard 后扫描器仍判通过（判据无牙）"
else
    ok "[1s] 负控：删掉 guard ⇒ 扫描器报警（判据有牙）"
fi

# ── [2] PX-005 base64 严格变体 ─────────────────────────────────────────
cat > "$TMP/p2.px" <<'PXEOF'
var r = []
r.append(base64_decode_strict("QQ") == null)
r.append(base64_decode_strict("QQ==") == "A")
r.append(base64_decode_strict("QQQ=") == "A" + chr(4))
r.append(base64_decode_strict("Q===") == null)
r.append(base64_decode_strict("QU*D") == null)
r.append(len(base64_decode_strict("")) == 0)
r.append(base64_decode_strict("QUJD\n") == "ABC")
r.append(base64_decode_strict("QUJD\r\nQUJD") == "ABCABC")
r.append(base64_to_bytes_strict("QQ") == null)
r.append(len(base64_to_bytes_strict("QUJD")) == 3)
r.append(base64_decode("QQ") == "A")
r.append(base64_decode("QQ=") == "A")
print(json_stringify(r))
PXEOF
b64l=$("$PX" run "$TMP/p2.px" 2>"$TMP/p2.err")
if [ "$b64l" = "[true,true,true,true,true,true,true,true,true,true,true,true]" ]; then
    ok "[2] 严格变体 10 条规则全中（拒 QQ/QQ=/Q===、收 QQ== 与 QQQ=、跳过 CRLF、非法字符→null）"
else
    bad "[2] 严格变体判据不符：$b64l"
fi
# [2 负控] 即 r[10]/r[11]：若有人把旧宽松函数也改成严格，这两条转红（旧语义冻结）

# ── [3] PX-011 json_stringify_go 的 \b \f ─────────────────────────────
cat > "$TMP/p3.px" <<'PXEOF'
print(json_stringify_go({"a": "x" + chr(8) + "y" + chr(12) + "z"}))
print(json_stringify_go({"a": chr(1) + chr(31)}))
print(len(json_stringify({"a": chr(8)})))
PXEOF
"$PX" run "$TMP/p3.px" > "$TMP/o3.txt" 2>/dev/null
line1=$(sed -n '1p' "$TMP/o3.txt"); line2=$(sed -n '2p' "$TMP/o3.txt"); line3=$(sed -n '3p' "$TMP/o3.txt")
[ "$line1" = '{"a":"x\by\fz"}' ] \
    && ok "[3] json_stringify_go 输出 \\b \\f（与 Go 实测逐字节一致）" \
    || bad "[3] json_stringify_go 仍不一致：$line1"
[ "$line2" = '{"a":"\u0001\u001f"}' ] \
    && ok "[3] 其余控制字符仍走 \\u00XX（未过度转义）" \
    || bad "[3] 控制字符转义变了：$line2"
# 负控：非 _go 那支仍输出裸字节（{"a":"} + 1 字节 + "} = 9）⇒ 证明只动了一支
[ "$line3" = "9" ] && ok "[3] 负控：json_stringify（非 _go）仍输出裸字节 ⇒ 只动了一支" \
    || bad "[3] 负控失败：json_stringify 也被改了（len=$line3）"

# ── [4] PX-014 词法转义 ───────────────────────────────────────────────
cat > "$TMP/p4.px" <<'PXEOF'
var s = "x\by\fz"
print(len(s))
print(s == "x" + chr(8) + "y" + chr(12) + "z")
print("\x41\x7a")
print("\v" == chr(11))
print(len("\x00"))
PXEOF
"$PX" run "$TMP/p4.px" > "$TMP/o4.txt" 2>"$TMP/e4.txt"; rc4=$?
if [ "$rc4" = 0 ]; then
    ok "[4] \\b \\f \\v \\xNN 均可编译（修前全是 E1005）"
else
    bad "[4] 新转义编译失败 rc=$rc4：$(head -2 "$TMP/e4.txt" | tr '\n' ' ')"
fi
[ "$(sed -n '1p' "$TMP/o4.txt")" = "5" ] && ok "[4] \"x\\by\\fz\" 长度 5（\\b/\\f 各 1 字节）" \
    || bad "[4] 长度不对：$(sed -n '1p' "$TMP/o4.txt")"
[ "$(sed -n '2p' "$TMP/o4.txt")" = "true" ] && ok "[4] 等值于 chr(8)/chr(12) 拼接" \
    || bad "[4] 与 chr() 拼接不等值"
[ "$(sed -n '3p' "$TMP/o4.txt")" = "Az" ] && ok "[4] \\xNN 逐字节可控（\\x41\\x7a → Az）" \
    || bad "[4] \\xNN 不对：$(sed -n '3p' "$TMP/o4.txt")"
[ "$(sed -n '4p' "$TMP/o4.txt")" = "true" ] && ok "[4] \\v == chr(11)" || bad "[4] \\v 不对"
[ "$(sed -n '5p' "$TMP/o4.txt")" = "1" ] && ok "[4] \\x00 是 1 字节（不是空串）" || bad "[4] \\x00 不对"
# 负控：非法转义仍须拒绝
printf 'var s = "\\q"\n' > "$TMP/p4n.px"
o4n=$("$PX" run "$TMP/p4n.px" 2>&1); rc4n=$?
if [ "$rc4n" != 0 ] && echo "$o4n" | grep -q E1005; then
    ok "[4] 负控：\\q 仍是 E1005（非法转义没被放行）"
else
    bad "[4] 负控失败：\\q 被放行（rc=$rc4n）"
fi

# ── [5] PX-024 write_bytes fd 直写 ────────────────────────────────────
cat > "$TMP/p5.px" <<PXEOF
write_bytes("/dev/stdout", bytes("line1\n"))
write_bytes("/dev/stdout", bytes("line2\n"))
write_bytes("$TMP/n.txt", bytes("first\n"))
write_bytes("$TMP/n.txt", bytes("second\n"))
write_bytes("/dev/stderr", bytes("to-stderr\n"))
PXEOF
"$PX" run "$TMP/p5.px" > "$TMP/out5.txt" 2>"$TMP/err5.txt"
n5=$(grep -c '^line[12]$' "$TMP/out5.txt")
[ "$n5" = "2" ] && ok "[5] 两条 /dev/stdout 写入重定向后**都在**（修前只剩最后一条）" \
    || bad "[5] 只剩 $n5 条 line（O_TRUNC 未修）"
# （不做 /dev/stderr 断言：管道下旧实现也能过，那条判据无牙 ⇒ 不写进来充数）
# 负控：普通文件路径必须仍保持 O_TRUNC（证明没顺手改掉既有语义）
if [ -f "$TMP/n.txt" ] && [ "$(cat "$TMP/n.txt")" = "second" ]; then
    ok "[5] 负控：普通路径写两次仍只留最后一次（O_TRUNC 语义原样保留）"
else
    bad "[5] 负控失败：普通文件语义被改（$(cat "$TMP/n.txt" 2>/dev/null | tr '\n' ' ')）"
fi
# [5s] 静态：fd 直写分支在函数体内（剥掉 // 注释行）
wb_body=$(awk '/static LXValue bi_write_bytes/,/^}/' runtime/runtime.c | grep -v '^[[:space:]]*//')
if echo "$wb_body" | grep -q 'strcmp(p, "/dev/stdout")' && echo "$wb_body" | grep -q 'write(fd,'; then
    ok "[5s] bi_write_bytes 内有 fd 直写分支（/dev/stdout · /dev/stderr · /dev/fd/N）"
else
    bad "[5s] fd 直写分支不见了（会退回 O_TRUNC 截断）"
fi

echo "──── M384 缺陷门：$pass 通过 / $fail 失败 ────"
[ "$fail" = 0 ] || { echo "❌ m384_px_defects 判红"; exit 1; }
echo "M384-DEFECTS-OK"
