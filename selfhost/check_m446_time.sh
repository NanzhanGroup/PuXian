#!/usr/bin/env bash
# ============================================================
# check_m446_time.sh —— 时区参数的**外部定标** + 非法输入必须响亮（M446 · 缺陷 869 同族）
# ------------------------------------------------------------
# 为什么有它：
#   2026-11-25 实测：time_format(0,"%H:%M","bogus") 与 ("+99") 都返回 "00:00"，
#   time_parse 的 %z 分支同样静默当 UTC。修复（M446）之后必须有人守着两件事：
#     ① 合法时区的**期望值不由本实现给出** —— 用 CPython 的 datetime/strftime 独立算
#        （M445 的「A==B==C==期望」范式，这里是 time 面的第一个落地）
#     ② 非法时区必须**响亮**（rc≠0 且报 R1002 指名），不是回落到 UTC
#
# 判据：
#   C1 oracle：CPython 算出 ≥6 条合法期望（行数不足 ⇒ 生成器坏了，rc=3）
#   C2 编译轨（pxc build 产物）逐行 == oracle
#   C3 解释轨（px run）逐行 == oracle（两条实现同源，但入口不同，防「只修一边」）
#   C4 非法时区 5 例（bogus / +99 / +24:00 / +08:60 / 空串）全部 rc≠0 且含 "非法时区"
#   N1 负控：篡改 oracle 一行 ⇒ C2 必须红（判据有牙）
#   G1 环境守卫：tools/pxc 或 tools/px 不可用 ⇒ rc=3
#
# 用法：check_m446_time.sh [--repo DIR]
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
while [ $# -gt 0 ]; do
  case "$1" in
    --repo) ROOT="$2"; shift 2 ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done

[ -x "$ROOT/tools/pxc" ] || { echo "ENV: 缺 tools/pxc"; exit 3; }
[ -x "$ROOT/tools/px" ]  || { echo "ENV: 缺 tools/px"; exit 3; }

TMP=$(mktemp -d /tmp/m446time.XXXXXX) || exit 3
trap 'rm -rf "$TMP"' EXIT
ok_n=0; bad_n=0
ok()  { echo "✅ $*"; ok_n=$((ok_n+1)); }
bad() { echo "❌ $*"; bad_n=$((bad_n+1)); }

# ── C1：CPython 独立算期望 ─────────────────────────────────
python3 - "$TMP/oracle.txt" <<'PY' || { echo "ENV: oracle 生成失败"; exit 3; }
import sys
from datetime import datetime, timezone, timedelta
cases = [("UTC", 0), ("Z", 0), ("+00:00", 0), ("+08:00", 28800),
         ("+0800", 28800), ("-05:30", -19800), ("+23:59", 86340)]
with open(sys.argv[1], "w") as f:
    for tz, off in cases:
        dt = datetime.fromtimestamp(0, timezone(timedelta(seconds=off)))
        f.write(dt.strftime("%Y-%m-%d %H:%M:%S") + "\n")
PY
N=$(wc -l < "$TMP/oracle.txt")
if [ "$N" -ge 6 ]; then ok "C1 oracle 期望 $N 行（CPython 独立算出）"; else bad "C1 oracle 只有 $N 行（<6）"; fi

# ── 被测程序（合法面）──────────────────────────────────────
cat > "$TMP/ok.px" <<'PX'
for tz in ["UTC", "Z", "+00:00", "+08:00", "+0800", "-05:30", "+23:59"]:
    print(time_format(0, "%Y-%m-%d %H:%M:%S", tz))
PX

if ! "$ROOT/tools/pxc" build "$TMP/ok.px" > "$TMP/build.log" 2>&1; then
  bad "C2 构建失败"; tail -5 "$TMP/build.log" | sed 's/^/     /'
else
  "$TMP/build/ok" > "$TMP/out_c.txt" 2>&1
  if diff -u "$TMP/oracle.txt" "$TMP/out_c.txt" > "$TMP/d1.txt"; then
    ok "C2 编译轨逐行 == CPython 期望（$N 行）"
  else
    bad "C2 编译轨与期望不一致"; sed -n '1,12p' "$TMP/d1.txt" | sed 's/^/     /'
  fi
fi

# ── C3 解释轨 ──────────────────────────────────────────────
if (cd "$ROOT" && ./tools/px run "$TMP/ok.px") > "$TMP/out_i.txt" 2>&1; then
  if diff -u "$TMP/oracle.txt" "$TMP/out_i.txt" > "$TMP/d2.txt"; then
    ok "C3 解释轨逐行 == CPython 期望"
  else
    bad "C3 解释轨与期望不一致"; sed -n '1,12p' "$TMP/d2.txt" | sed 's/^/     /'
  fi
else
  bad "C3 解释轨执行失败（rc≠0）"
fi

# ── C4 非法时区必须响亮 ───────────────────────────────────
bad_tz=0
i=0
for tz in bogus +99 +24:00 +08:60 ""; do
  i=$((i+1))
  cat > "$TMP/bad$i.px" <<PX
print(time_format(0, "%H:%M", "$tz"))
PX
  if (cd "$ROOT" && ./tools/px run "$TMP/bad$i.px") > "$TMP/bad$i.out" 2>&1; then
    echo "     ⚠️ 时区 [$tz] 未报错（rc=0）"; bad_tz=$((bad_tz+1))
  elif grep -q '非法时区' "$TMP/bad$i.out"; then
    :
  else
    echo "     ⚠️ 时区 [$tz] 报错但文案未指名：$(head -1 "$TMP/bad$i.out")"; bad_tz=$((bad_tz+1))
  fi
done
if [ "$bad_tz" = 0 ]; then ok "C4 非法时区 5 例全部响亮且指名"; else bad "C4 有 $bad_tz 例未按契约报错"; fi

# ── N1 负控：篡改一行期望 ⇒ C2 必须红 ─────────────────────
sed '1s/.*/1970-01-01 00:00:01/' "$TMP/oracle.txt" > "$TMP/oracle_bad.txt"
if [ -f "$TMP/out_c.txt" ] && diff -q "$TMP/oracle_bad.txt" "$TMP/out_c.txt" >/dev/null 2>&1; then
  bad 'N1 负控：篡改期望后仍一致 ⇒ 判据没牙'
else
  ok "N1 负控：篡改期望一行 ⇒ 比对必红"
fi

echo "M446-TIME-OK pass=$ok_n fail=$bad_n"
[ "$bad_n" = 0 ] && exit 0 || exit 1
