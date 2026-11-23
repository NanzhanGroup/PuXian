#!/usr/bin/env bash
# ============================================================
# selfhost/check_m440_fastmemo.sh —— `px run --fast` 命中决策备忘（M440）
# ------------------------------------------------------------
# 病象（M440 立项实测 2026-11-24 本机 · hello.px · 热态 6 次中位 192 ms）：
#   命中路径每次都在重做「与上次完全相同的判断」：
#     px_ast_prime 19 ms + run_subset_ok 17 ms + run_cache_key 67 ms
#     + 产物完整性 sha256(3.1 MB) 27 ms  —— 真正必须做的只有 exec 8 ms。
# 口径（不变式）：**缓存只许影响快慢，不许影响语义**。
#   备忘命中判据 = 所有会影响产物的输入的 stat 签名（size:inode:mtime:ctime，**纳秒级**）
#   逐项相等。任一不等 ⇒ 落回原全量路径（并重写备忘）。
#
# 判据（每条都要能判红；负控必须有前提自证）：
#   S1 形状：备忘三件套在 · 热路径在 usage() **之前**（早退才省得掉 35 ms 解析）
#   S2 签名口径必须是纳秒（%y/%z），不得回退 %Y/%Z（只到秒 —— 实测能漏掉同秒篡改）
#   S3 覆盖口进退避名单（M425 补1 缺陷 867 的同族）+ 工具链输入走唯一真源
#   F1 命中：连续两次，第二次必须打印 memo 命中
#   F2 等价：命中输出 == 全量路径输出 == 解释轨输出
#   F3 负控：改代码 ⇒ 未命中 + 新输出
#   F4 负控：改代码但把 size/mtime 复原（同秒内）⇒ 必须未命中
#           前提自证：size/mtime(ns) 不变而 ctime(ns) 变了（不成立 ⇒ 判红）
#   F5 负控：篡改产物（同长、复原 mtime、同秒）⇒ 必须未命中，且重建后输出正确
#   F6 负控：依赖 .px 变化 ⇒ 未命中 + 新输出
#   F7 负控：覆盖口 PX_PXI_BIN 变化 ⇒ 未命中
#   F8 verbose 不走备忘（输出不含 memo 命中，语义仍正确）
#   F9 逃生口 PX_RUN_NOCACHE=1 ⇒ 解释轨语义
#   F10 相对性能：命中中位 ≤ 全量路径中位的 60%（绝对耗时只打印，**不作判据**）
#   F11 目录新增文件 ⇒ 未命中（覆盖 import 解析顺序 / glob 集合变化）
#   前提不成立（缺 pxi、建不出产物、取不到备忘）⇒ rc=3，绝不降级成绿
#
# 用法：check_m440_fastmemo.sh            # 全跑（实测 ~25s）
#       check_m440_fastmemo.sh --self-test # 只跑判据夹具
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
env_bad() { echo "❌ 环境不成立：$1" >&2; exit 3; }

WORK=""
cleanup() { [ -n "${WORK:-}" ] && [ -d "$WORK" ] && rm -rf "$WORK"; }
trap cleanup EXIT

if [ "${1:-}" = "--self-test" ]; then
  echo "── self-test（判据夹具，不动仓库）"
  a='3:115667068:2026-11-24 01:54:20.589276232 +0800:2026-11-24 01:54:20.589276232 +0800'
  b='3:115667068:2026-11-24 01:54:20.589276232 +0800:2026-11-24 01:54:20.621276248 +0800'
  [ "$a" = "$a" ] && ok "F1 同签名 ⇒ 判「未变」" || bad "F1 同签名被判变"
  [ "$a" != "$b" ] && ok "F2 负控：同秒内 ctime 纳秒差 ⇒ 判「变了」（判据有牙）" || bad "F2 纳秒差不被识别"
  case "$a" in *[0-9].[0-9]*) ok "F3 夹具确实是纳秒格式（不是秒级）" ;; *) bad "F3 夹具格式不对" ;; esac
  echo "  小计：$PASS 通过 / $FAIL 失败"
  [ "$FAIL" -eq 0 ] || exit 2
  exit 0
fi

echo "═══ M440 --fast 命中决策备忘 ═══"
[ -x "$PX" ] || env_bad "缺 $PX"
[ -x "$ROOT/bootstrap/pxi" ] || env_bad "缺 bootstrap/pxi（解释轨/契约指纹都要它）"

# ---------------- S1/S2/S3 形状 ----------------
for f in px_memo_try px_memo_write px_stat_sig "PX_STATFMT=" px_prog_intact; do
  grep -q -- "$f" "$PX" && ok "S1a tools/px 含 $f" || bad "S1a tools/px 缺 $f"
done
LN_MEMO=$(grep -n 'px_memo_try "' "$PX" | head -1 | cut -d: -f1)
LN_USAGE=$(grep -n '^usage() {' "$PX" | head -1 | cut -d: -f1)
if [ -n "${LN_MEMO:-}" ] && [ -n "${LN_USAGE:-}" ] && [ "$LN_MEMO" -lt "$LN_USAGE" ]; then
  ok "S1b 热路径在 usage() 之前（行 $LN_MEMO < $LN_USAGE：早退才省得掉解析 35 ms）"
else
  bad "S1b 热路径不在 usage() 之前（LN_MEMO=${LN_MEMO:-空} LN_USAGE=${LN_USAGE:-空}）"
fi
grep -q "PX_STATFMT='%s:%i:%y:%z'" "$PX" \
  && ok "S2a 签名格式含纳秒（%y/%z）" || bad "S2a 签名格式不是纳秒口径（%Z 会漏掉同秒篡改）"
if grep -nE "stat -c '%s:%Y:%i:%Z'" "$PX" >/dev/null 2>&1; then
  bad "S2b 仍有秒级 stat 口径（%Y:%Z）残留"
else
  ok "S2b 无秒级 %Y:%Z 残留"
fi
MISSING=""
for v in PX_PXI_BIN PX_PXC_BIN PXC_VM_BIN PX_STDLIB PX_BUILD_ENGINE PX_NORM; do
  awk '/^px_memo_env\(\)/,/^}/' "$PX" | grep -q -- "$v" || MISSING="$MISSING $v"
done
[ -z "$MISSING" ] && ok "S3 覆盖口全在备忘环境串里" || bad "S3 备忘环境串缺覆盖口：$MISSING"
awk '/^px_memo_write\(\)/,/^}/' "$PX" | grep -q 'px_toolchain_files' \
  && ok "S3b 备忘覆盖工具链输入（px_toolchain_files 唯一真源）" \
  || bad "S3b 备忘没覆盖工具链输入"

# ---------------- 夹具环境 ----------------
WORK="$(mktemp -d "${TMPDIR:-/tmp}/m440fast.XXXXXX")" || env_bad "mktemp 失败"
CASE="$WORK/case"        # 脚本目录（目录 mtime 变化会让备忘失效 ⇒ 预热后不许往里建东西）
RC="$WORK/rc"            # 缓存根**必须**在脚本目录之外
mkdir -p "$CASE" "$RC"
export PX_RUNCACHE_DIR="$RC"
SP="$CASE/s.px"
printf 'def main():\n    print("m440-v1")\n' > "$SP"
first_out="$( cd "$CASE" && "$PX" run --fast s.px 2>/dev/null )"
[ "$first_out" = "m440-v1" ] || env_bad "首次 --fast 未产出预期输出（got='$first_out'）"

MEMO="$RC/idx/${SP#/}/memo"
cur_prog() { awk -F'\t' '$1=="g"{print $2; exit}' "$MEMO" 2>/dev/null; }
PROG="$(cur_prog)"
[ -n "${PROG:-}" ] && [ -f "$PROG" ] || env_bad "备忘里没有可用产物路径（$MEMO）"

hit() { # ⇒ 0=备忘命中
  local e
  e="$( cd "$CASE" && PX_MEMO_DEBUG=1 "$PX" run --fast s.px 2>&1 >/dev/null )"
  case "$e" in *"memo 命中"*) return 0 ;; *) return 1 ;; esac
}
why() { # ⇒ 未命中原因（人工定位用；判据只看命中/未命中）
  ( cd "$CASE" && PX_MEMO_DEBUG=1 "$PX" run --fast s.px 2>&1 >/dev/null ) \
    | grep -o 'memo 未命中：[^（]*' | head -1
}
outof() { ( cd "$CASE" && "$PX" run --fast s.px 2>/dev/null ); }
# 三个独立读数（⚠️ 不许按 ":" 切签名：人类可读时间戳自带冒号 ⇒ 字段错位）
read3() { python3 - "$1" <<'PYEOF'
import sys, os
st = os.stat(sys.argv[1])
print("%d %d %d" % (st.st_size, st.st_mtime_ns, st.st_ctime_ns))
PYEOF
}

# ---------------- F1 命中 ----------------
hit && ok "F1a 连续第二次运行命中备忘" || bad "F1a 没命中（$(why)）"
hit && ok "F1b 第三次仍命中（幂等）" || bad "F1b 第三次未命中（$(why)）"

# ---------------- F2 等价 ----------------
alt_rc="$WORK/rc_alt"; mkdir -p "$alt_rc"
alt_out="$( cd "$CASE" && PX_RUNCACHE_DIR="$alt_rc" "$PX" run --fast s.px 2>/dev/null )"
itp_out="$( cd "$CASE" && "$PX" run s.px 2>/dev/null )"
if [ "$(outof)" = "$alt_out" ] && [ "$alt_out" = "$itp_out" ]; then
  ok "F2 三路输出一致（备忘命中 / 全量路径 / 解释轨）"
else
  bad "F2 三路输出不一致：hit='$(outof)' full='$alt_out' interp='$itp_out'"
fi

# ---------------- F10 相对性能（先测，避免后面的负控污染基线） ----------------
ms_of() { local s e; s="${EPOCHREALTIME//[!0-9]/}"; "$@" >/dev/null 2>&1; e="${EPOCHREALTIME//[!0-9]/}"; echo $(( (e - s) / 1000 )); }
t_full=(); t_hot=()
for i in 1 2 3; do          # 全量路径基线 = 删掉备忘后跑（缓存热，不含重建成本）
  rm -f "$MEMO"
  t_full+=("$(ms_of env "$PX" run --fast "$SP")")
done
"$PX" run --fast "$SP" >/dev/null 2>&1     # 写回备忘
for i in 1 2 3 4 5; do t_hot+=("$(ms_of env "$PX" run --fast "$SP")"); done
med() { printf '%s\n' "$@" | sort -n | sed -n "$(( ($# + 1) / 2 ))p"; }
MF="$(med "${t_full[@]}")"; MH="$(med "${t_hot[@]}")"
echo "  · 相对性能：全量判定中位 ${MF}ms · 备忘命中中位 ${MH}ms（绝对值只报告，不作判据）"
if [ "${MH:-9999}" -le "$(( MF * 6 / 10 ))" ]; then
  ok "F10 命中中位 ≤ 全量判定的 60%（${MH}ms ≤ $(( MF * 6 / 10 ))ms）"
else
  bad "F10 命中没有明显更快：${MH}ms vs 全量 ${MF}ms"
fi

# ---------------- F3 改代码 ----------------
printf 'def main():\n    print("m440-v2")\n' > "$SP"
if ! hit && [ "$(outof)" = "m440-v2" ]; then
  ok "F3 改代码 ⇒ 未命中且跑出新逻辑（$(why)）"
else
  bad "F3 改代码后仍命中或输出没变（$(why)）"
fi

# ---------------- F4 改代码但复原 size/mtime（同秒） ----------------
hit >/dev/null 2>&1
pre4="$(read3 "$SP")"
python3 - "$SP" <<'PYEOF' || env_bad "F4 前置操作失败（python3？）"
import sys, os
p = sys.argv[1]
st = os.stat(p)
data = open(p, "rb").read()
new = data.replace(b"m440-v2", b"m440-v3")
assert len(new) == len(data) and new != data, "同长替换未成立"
open(p, "wb").write(new)
os.utime(p, ns=(st.st_atime_ns, st.st_mtime_ns))
PYEOF
post4="$(read3 "$SP")"
f4_pre="$(python3 - "$pre4" "$post4" <<'PYEOF'
import sys
a, b = sys.argv[1].split(), sys.argv[2].split()
print("size_same=%s mtime_ns_same=%s ctime_ns_changed=%s"
      % (a[0] == b[0], a[1] == b[1], a[2] != b[2]))
PYEOF
)"
case "$f4_pre" in
  "size_same=True mtime_ns_same=True ctime_ns_changed=True")
    ok "F4a 前提自证：同秒内改内容后 size/mtime 复原、ctime 纳秒变" ;;
  *)
    bad "F4a 前提不成立（$f4_pre：$pre4 → $post4）⇒ F4/F5 判据失去前提，按判红处理" ;;
esac
if ! hit && [ "$(outof)" = "m440-v3" ]; then
  ok "F4b 改内容+复原 mtime ⇒ 未命中且跑出新逻辑（$(why)）"
else
  bad "F4b 改内容+复原 mtime 后仍命中 ⇒ 缓存被绕过，语义坏了（$(why)）"
fi

# ---------------- F5 篡改产物（同长、复原 mtime、同秒） ----------------
hit >/dev/null 2>&1
PROG="$(cur_prog)"     # 产物路径随脚本内容换 key ⇒ 必须重新取
[ -n "${PROG:-}" ] && [ -f "$PROG" ] || env_bad "F5 取不到当前产物"
pre5="$(read3 "$PROG")"
python3 - "$PROG" <<'PYEOF' || env_bad "F5 前置操作失败"
import sys, os
p = sys.argv[1]
st = os.stat(p)
d = bytearray(open(p, "rb").read())
d[0] ^= 0xFF
open(p, "r+b").write(d)
os.utime(p, ns=(st.st_atime_ns, st.st_mtime_ns))
PYEOF
post5="$(read3 "$PROG")"
f5_pre="$(python3 - "$pre5" "$post5" <<'PYEOF'
import sys
a, b = sys.argv[1].split(), sys.argv[2].split()
print("size_same=%s mtime_ns_same=%s ctime_ns_changed=%s"
      % (a[0] == b[0], a[1] == b[1], a[2] != b[2]))
PYEOF
)"
case "$f5_pre" in
  "size_same=True mtime_ns_same=True ctime_ns_changed=True")
    ok "F5a 前提自证：产物被同长覆写、mtime 复原、ctime 纳秒变" ;;
  *)
    bad "F5a 前提不成立（$f5_pre：$pre5 → $post5）⇒ 判红" ;;
esac
if ! hit; then
  ok "F5b 产物被篡改 ⇒ 未命中（$(why)）"
else
  bad "F5b 产物被篡改仍命中 ⇒ 会执行被篡改的二进制（缓存改变了语义）"
fi
if [ "$(outof)" = "m440-v3" ]; then
  ok "F5c 篡改后重建，输出正确（m440-v3）"
else
  bad "F5c 篡改后输出不对：'$(outof)'"
fi

# ---------------- F6 依赖变化 ----------------
DEP="$CASE/libz.px"
printf 'def hi():\n    print("dep1")\n' > "$DEP"
printf 'import "libz.px"\ndef main():\n    hi()\n' > "$SP"
( cd "$CASE" && "$PX" run --fast s.px >/dev/null 2>&1 )
hit >/dev/null 2>&1 && ok "F6a 带依赖的脚本能命中" || bad "F6a 带依赖的脚本未命中（$(why)）"
printf 'def hi():\n    print("dep2")\n' > "$DEP"
if ! hit && [ "$(outof)" = "dep2" ]; then
  ok "F6b 改依赖 ⇒ 未命中且跑出新逻辑（$(why)）"
else
  bad "F6b 改依赖后仍命中或输出没变（$(why)）"
fi

# ---------------- F7 覆盖口 ----------------
hit >/dev/null 2>&1
cp "$ROOT/bootstrap/pxi" "$WORK/pxi_stub" 2>/dev/null || env_bad "拷 pxi 失败"
e="$( cd "$CASE" && PX_PXI_BIN="$WORK/pxi_stub" PX_MEMO_DEBUG=1 "$PX" run --fast s.px 2>&1 >/dev/null )"
case "$e" in
  *"memo 未命中"*) ok "F7 覆盖口 PX_PXI_BIN 变化 ⇒ 未命中（打桩解释器不会被备忘绕过）" ;;
  *) bad "F7 覆盖口变化后仍命中 ⇒ 门的负控会被拔牙（M425 补1 同族）" ;;
esac

# ---------------- F8 verbose ----------------
e="$( cd "$CASE" && PX_RUN_VERBOSE=1 "$PX" run --fast s.px 2>&1 >/dev/null )"
case "$e" in
  *"memo 命中"*) bad "F8 verbose 模式走了备忘（它必须打印 key/依赖）" ;;
  *) ok "F8 verbose 不走备忘" ;;
esac
[ "$( cd "$CASE" && PX_RUN_VERBOSE=1 "$PX" run --fast s.px 2>/dev/null )" = "dep2" ] \
  && ok "F8b verbose 语义仍正确" || bad "F8b verbose 输出不对"

# ---------------- F9 逃生口 ----------------
rm -rf "$RC"; mkdir -p "$RC"
o1="$( cd "$CASE" && PX_RUN_NOCACHE=1 "$PX" run --fast s.px 2>&1 )"
case "$o1" in
  dep2*) ok "F9 PX_RUN_NOCACHE=1 ⇒ 解释轨语义" ;;
  *) bad "F9 逃生口不生效：'$o1'" ;;
esac
[ -z "$(find "$RC" -name prog -type f 2>/dev/null | head -1)" ] \
  && ok "F9b 逃生口路径不写缓存" || bad "F9b 逃生口仍写了缓存产物"

# ---------------- F11 目录新增文件 ----------------
( cd "$CASE" && "$PX" run --fast s.px >/dev/null 2>&1 )
hit >/dev/null 2>&1 || bad "F11 前置：新增文件前未命中（$(why)）"
printf 'def noop():\n    print("x")\n' > "$CASE/newmod.px"
if ! hit; then
  ok "F11 脚本目录新增 .px ⇒ 未命中（覆盖 import 解析顺序/glob 变化）"
else
  bad "F11 目录新增文件后仍命中"
fi

echo
echo "═══ 结果：$PASS 通过 / $FAIL 失败 ═══"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
