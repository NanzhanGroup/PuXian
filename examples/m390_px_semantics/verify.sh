#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M390 · 清歌 / 晨曦 运行时缺陷收口（缺陷 713–729）
# ---------------------------------------------------------------------------
# 判据分六组，**每组都配一条「若修法回退则必红」的差分断言**（不靠固定字面量行窗口）：
#   A NUL 统一     split/replace/contains/trim 对含 0x00 的值按字节长处理
#   B 惰性量词     `.*?` 与 `.*` 的**结果必须不同**（lazy 位被忽略 ⇒ 二者相等 ⇒ 红）
#   C 严格 JSON    尾随垃圾 / 前导零 / `1.` / `.5` / `+1` 必须 Err；合法输入必须 Ok
#   D Unicode      U+00A0 / U+3000 / 希腊 / 西里尔；对照 ASCII 回归
#   E 新原语        sha256_raw · hkdf(RFC5869) · AAD · list_remove_at · bytes_find(start)
#                  · os_spawn_capture_sep · int_floor/int_ceil/trunc · read_chunk_str
#   F 时区         无 tz = 本地 · "UTC" = UTC · 三条路径互不相同（TZ 非 UTC 机器上）
# 前提自证：编译产物里 native_symbols() 必须含本轮新增的 9 个名字（证明链的是当前 runtime，
#          而不是上一个版本的缓存 .o）。
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
TMP="${TMPDIR:-/tmp}/m390_pxsem.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

# 含 NUL 的样本：a \0 b \n c \n （6 字节）
printf 'a\0b\nc\n' > "$TMP/nul.bin"

cat > "$TMP/surface.px" <<'PXEOF'
let NULP = __NULPATH__
let s = read_file(NULP)
print("A1 " + str(len(s)))
print("A2 " + str(len(split(s, "\n"))))
print("A3 " + str(len(replace(s, chr(0), "XX"))))
print("A4 " + str(contains(s, chr(0))))
print("A5 " + str(len(trim(chr(160) + "x" + chr(160)))))
print("A6 " + to_upper("äöü"))
print("A7 " + to_upper("привет"))
print("A8 " + str(starts_with(s, "a")) + "," + str(ends_with(s, "c\n")))
print("B1 " + str(regex_match("a.*?b", "axxbxxb")))
print("B2 " + regex_find("a.*?b", "axxbxxb"))
print("B3 " + regex_find("a.*b", "axxbxxb"))
print("B4 " + str(regex_find_all("a.*?c", "xabcx")))
print("B5 " + regex_replace("<(.*?)>", "<a><b>", "[$1]"))
print("B6 " + str(regex_find_all("a.*c", "xabcx")))
print("C1 " + str(json_parse_opt("{\"a\":1}x").is_err()))
print("C2 " + str(json_parse_opt("{\"a\":01}").is_err()))
print("C3 " + str(json_parse_opt("{\"a\":1.}").is_err()))
print("C4 " + str(json_parse_opt("{\"a\":.5}").is_err()))
print("C5 " + str(json_parse_opt("{\"a\":+1}").is_err()))
print("C6 " + str(json_parse("{\"a\":1,\"b\":[1,2]}")))
print("C7 " + str(json_parse_opt("not json").is_err()))
print("D1 " + str(len(trim(chr(12288) + "y"))))
print("D2 " + to_lower("ÄÖÜ"))
print("D3 " + to_upper("abc"))
print("E1 " + str(len(sha256_raw(""))) + " " + bytes_to_hex(sha256_raw("")))
print("E2 " + bytes_to_hex(hkdf_sha256(hex_to_bytes("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b"), hex_to_bytes("000102030405060708090a0b0c"), hex_to_bytes("f0f1f2f3f4f5f6f7f8f9"), 42)))
print("E3 " + str(len(hmac_sha256_raw("k", "m"))))
print("E4 " + str(aes_gcm_decrypt(aes_gcm_encrypt("hello", "0123456789abcdef", "0123456789ab", "hdr"), "0123456789abcdef", "0123456789ab", "hdr")))
print("E5 " + str(aes_gcm_decrypt(aes_gcm_encrypt("hello", "0123456789abcdef", "0123456789ab", "hdr"), "0123456789abcdef", "0123456789ab", "xx")))
print("E6 " + str(list_remove_at([1,2,3,4], 1)))
print("E7 " + str(bytes_find(bytes("abcabc"), bytes("abc"), 1)))
print("E8 " + str(int_floor(-7.9)) + "," + str(int_ceil(-7.9)) + "," + str(trunc(-7.9)))
print("E9 " + os_spawn_capture_sep("sh", ["-c", "echo o; echo e >&2"]).get("stderr"))
print("E10 " + str(len(read_chunk_str(open(NULP, "r"), 4))))
print("F1 " + time_format(0, "%H:%M") + "|" + time_format(0, "%H:%M", "UTC"))
print("G1 " + str(contains(native_symbols(), "read_chunk_str")) + str(contains(native_symbols(), "hkdf_sha256")) + str(contains(native_symbols(), "list_remove_at")) + str(contains(native_symbols(), "int_floor")) + str(contains(native_symbols(), "sha256_raw")) + str(contains(native_symbols(), "os_spawn_capture_sep")) + str(contains(native_symbols(), "getcwd")) + str(contains(native_symbols(), "readlink")) + str(contains(native_symbols(), "trunc")))
PXEOF
sed -i "s|__NULPATH__|\"$TMP/nul.bin\"|" "$TMP/surface.px"

if ! "$PX" build "$TMP/surface.px" > "$TMP/build.log" 2>&1; then
  echo "❌ 编译失败："; tail -20 "$TMP/build.log"; exit 2
fi
OUT="$TMP/surface.out"
"$TMP/build/surface" > "$OUT" 2>&1
got() { sed -n "s/^$1 //p" "$OUT" | head -1; }
expect() { # name expected 说明
  local g; g="$(got "$1")"
  if [ "$g" = "$2" ]; then ok "$1 = $g（$3）"; else bad "$1 期望 [$2] 实得 [$g]（$3）"; fi
}
expect_ne() { # name 不能等于
  local g; g="$(got "$1")"
  if [ -n "$g" ] && [ "$g" != "$2" ]; then ok "$1 = $g（≠ $2 ⇒ 修复在位）"; else bad "$1 仍是 [$2]（回归）"; fi
}

expect A1 "6" "len 按字节长"
expect A2 "3" "split 遇 NUL 不再截断（修前 1）"
expect A3 "7" "replace 遇 NUL 不再截断（6-1+2=7）"
expect A4 "true" "contains 认内嵌 NUL"
expect A5 "1" "trim 认 U+00A0"
expect A6 "ÄÖÜ" "to_upper 拉丁扩展"
expect A7 "ПРИВЕТ" "to_upper 西里尔"
expect A8 "true,true" "starts/ends 字节长"

expect B1 "true" "惰性量词能匹配（修前 false）"
expect B2 "axxb" "惰性取最短"
expect B3 "axxbxxb" "贪婪取最长"
expect_ne B2 "axxbxxb"   # 差分：lazy 位被忽略 ⇒ 与贪婪相同 ⇒ 红
expect B4 "[abc]" "find_all 惰性（修前 []）"
expect B5 "[a][b]" "replace 惰性逐项（修前原样返回）"
expect_ne B4 "[]"

for c in C1 C2 C3 C4 C5 C7; do expect "$c" "true" "畸形 JSON 必须 Err"; done
expect C6 "{a: 1, b: [1, 2]}" "合法 JSON 仍可用"

expect D1 "1" "trim 认 U+3000"
expect D2 "äöü" "to_lower Unicode"
expect D3 "ABC" "ASCII 大小写回归"

expect E1 "32 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" "sha256_raw 返回 32 字节且等于空体摘要"
expect E2 "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865" "RFC 5869 Test Case 1 逐字节"
expect E3 "32" "hmac_sha256_raw 32 字节"
expect E4 "hello" "AAD 往返"
expect E5 "null" "AAD 不符必须解不开"
expect E6 "[1, 3, 4]" "list_remove_at"
expect E7 "3" "bytes_find 起始偏移"
expect E8 "-8,-7,-7" "int_floor/-ceil/trunc 负值三向（floor(-7.9)=-8 · ceil=-7 · trunc=-7）"
expect E9 "$(printf 'e')" "spawn_capture_sep 分离 stderr"
expect E10 "4" "read_chunk_str 4 字节"

# F：本机 TZ 若是 UTC，两条相等是**正确**的 —— 前提自证后按实际情况判
TZOFF="$("$TMP/build/surface" 2>/dev/null | sed -n 's/^F1 //p')"
L="$(printf '%s' "$TZOFF" | cut -d'|' -f1)"; U="$(printf '%s' "$TZOFF" | cut -d'|' -f2)"
if [ "$U" = "00:00" ]; then ok "F1 UTC 路径 = 00:00"; else bad "F1 UTC 路径异常：$U"; fi
if [ "$(date +%z)" = "+0000" ]; then
  ok "F1 本机 TZ=UTC ⇒ 本地与 UTC 相等属正确（前提已自证）"
else
  if [ "$L" != "$U" ]; then ok "F1 无 tz=$L ≠ UTC=$U（本地时区生效）"; else bad "F1 无 tz 仍等于 UTC（本地化未生效）"; fi
fi

# G：前提自证 —— 链的必须是当前 runtime（9 个新名全在）
expect G1 "truetruetruetruetruetruetruetruetrue" "native_symbols 含本轮 9 个新名（前提自证）"

# H：入库冒烟（**覆盖面台账的消费面**）
#   M250 台账只扫 `.px` 源文件，本门主语料是运行期生成到 /tmp 的 ⇒ 台账看不见 ⇒
#   M390 的 11 个新 native 全被判「新增未覆盖」（10-11 08:30 全量门实测）。
#   ⇒ 另提交 surface.px 并在本段**真实执行**（11 个 native 各合法调用一次），不留死文本。
if outsm="$("$PX" run "$ROOT/examples/m390_px_semantics/surface.px" 2>&1)"; then
  case "$outsm" in *M390-NATIVE-SMOKE-OK*) ok "H1 入库冒烟 surface.px 通过（11 个新 native 各调一次）" ;;
    *) bad "H1 入库冒烟未打印 OK 标记：$outsm" ;; esac
else
  bad "H1 入库冒烟 surface.px 失败：$(printf '%s' "$outsm" | tail -3)"
fi

echo
echo "M390 判据：$pass 过 / $fail 红"
[ "$fail" -eq 0 ] || exit 1
