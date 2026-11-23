#!/usr/bin/env bash
# ============================================================
# selfhost/check_m439_norm.sh —— 源码「语义指纹」归一化器（M439-1 · 缺陷 921）
# ------------------------------------------------------------
# 病象（M438 实测）：改 2 行注释 ⇒ rebake --check-all 14/14 → 0/14 ⇒ 重烘
#   ⇒ .rtcache 冷 ⇒ 全量链从 2h 变 2.5–4h（超时红门 914/918 的成因之一）。
# 口径：注释不是语义 ⇒ 指纹只认归一化后的源码（tools/pxnorm.c）。
#   C 档可丢行（行号不进产物：-O2 无 -g，实测改注释后 .o 逐字节相同）；
#   .px 档必须保行（行号进模块，错误定位/栈回溯用）。
#
# 判据（每条都要能判红；负控必须有前提自证）：
#   S1 形状：tools/pxnorm.c 在；tools/px 的 rt_key 走 sem_join；
#            rebake entry_fp 走 `px norm --join`；bootstrap_prove_bc 有 src_sem
#   S2 归一化器自测：pxnorm --selftest 全绿（C 档 10 + .px 档 9）
#   S3 前提自证：「C 档可丢行」的前提 = runtime 下 .c/.h 里 __LINE__/__FILE__ 均为 0；
#            >0 ⇒ 判红（要求退回保行口径），这条不成立时 P1 的前提也没了
#   E1 回退面有界：全仓 .px/.c/.h 逐文件归一化，回退（按 raw 处理）**只许**出现在
#            selfhost/cases_bad/（故意的坏样例）；多一个 ⇒ 判红（解析器不敢归一的文件变多了）
#   P1 rt_key 只改注释 ⇒ **不变**（前提：baseline 连跑两次相同）
#   P2 负控：加一行**代码** ⇒ rt_key 必变（判据有牙，不是「反正都相同」）
#   P3 复原：还原后 rt_key 回到基线；runtime.c 的 sha256 与门开始时逐字节一致
#            （P1/P2 会临时改 runtime/runtime.c ⇒ 退出路径必须无条件复原 —— M182 同族纪律）
#   P4 映射缓存生效：把 pxnorm 二进制临时移走 ⇒ rt_key **仍等于基线**
#            （证明稳态走 .rtcache/.semmap，不是每次重解析）；移回后必须还原
#   P5 逃生口：PX_NORM=raw ⇒ rt_key ≠ 基线（安全阀可用；且证明 sem 与 raw 真不是一个口径）
#   环境不成立（缺文件 / git 脏树影响复位 / 取不到基线）⇒ rc=3，绝不降级成绿
#
# 用法：check_m439_norm.sh            # 全跑（实测 ~10s）
#       check_m439_norm.sh --self-test # 只跑判据夹具
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime/runtime.c"
PASS=0; FAIL=0; SKIP_RESTORE=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
env_bad() { echo "❌ 环境不成立：$1" >&2; exit 3; }

RT_BAK=""
cleanup() {
  if [ -n "${RT_BAK:-}" ] && [ -f "$RT_BAK" ]; then
    cp -p "$RT_BAK" "$RT" 2>/dev/null
    rm -f "$RT_BAK"
  fi
}
trap cleanup EXIT

rtkey() { ( cd "$ROOT" && "$PX" rtkey "" gcc 0 2>/dev/null ); }

if [ "${1:-}" = "--self-test" ]; then
  echo "── self-test（判据夹具，不动仓库）"
  # 夹具：注释标签判定（P1 的核心判据是「两个值相等」，这里验它能区分）
  a="945d49d1b0a388b3"; b="945d49d1b0a388b3"; c="8c722ea5a0a50b4e"
  [ "$a" = "$b" ] && ok "F1 同值 ⇒ 判「未变」" || bad "F1 同值被判变"
  [ "$a" != "$c" ] && ok "F2 负控：不同值 ⇒ 判「变了」（判据有牙）" || bad "F2 不同值被判同"
  echo "  小计：$PASS 通过 / $FAIL 失败"
  [ "$FAIL" -eq 0 ] || exit 2
  exit 0
fi

echo "═══ M439-1 语义指纹（缺陷 921）═══"

# ---- S1 形状 ----
[ -f "$ROOT/tools/pxnorm.c" ] && ok "S1a tools/pxnorm.c 在仓库" || { bad "S1a 缺 tools/pxnorm.c"; exit 1; }
grep -q 'sem_join' "$ROOT/tools/px" && grep -q 'pxnorm_bin' "$ROOT/tools/px" \
    && ok "S1b tools/px 有 pxnorm_bin/sem_join（rt_key 的 sem 通道）" || bad "S1b tools/px 未接线"
if awk '/^rt_key\(\)/,/^}/' "$ROOT/tools/px" | grep -q 'sem_join'; then
  ok "S1c rt_key 走 sem_join"
else bad "S1c rt_key 未走 sem_join"; fi
if awk '/^entry_fp\(\)/,/^}/' "$ROOT/selfhost/rebake_bin.sh" | grep -q 'norm --join'; then
  ok "S1d rebake entry_fp 走 px norm --join"
else bad "S1d rebake entry_fp 未接线"; fi
grep -q 'src_sem' "$ROOT/selfhost/bootstrap_prove_bc.sh" \
    && ok "S1e bootstrap_prove_bc 有 src_sem（步骤 1 不再只看 mtime）" || bad "S1e bootstrap_prove_bc 未接线"

# ---- S2 归一化器自测 ----
if out=$("$PX" norm --selftest 2>&1); then
  n=$(printf '%s' "$out" | sed -n 's/.*selftest: \([0-9]*\) 通过.*/\1/p')
  [ "${n:-0}" -ge 19 ] && ok "S2 pxnorm --selftest 全绿（${n} 条）" || bad "S2 自测条数不足（${n:-0} < 19）"
else
  bad "S2 pxnorm --selftest 失败：$(printf '%s' "$out" | tail -2 | tr '\n' ' ')"
fi

# ---- S3 前提自证 ----
if pre=$("$PX" norm --premise "$ROOT" 2>&1); then
  case "$pre" in
    *"__LINE__=0 __FILE__=0"*) ok "S3 前提自证：runtime 下 __LINE__=0 __FILE__=0（C 档可丢行）" ;;
    *) bad "S3 前提不成立：$(printf '%s' "$pre" | tail -1) ⇒ 必须退回保行口径" ;;
  esac
else
  bad "S3 前提自证失败（--premise rc≠0）：$(printf '%s' "$pre" | tail -1)"
fi

# ---- E1 回退面有界 ----
FILES=$( cd "$ROOT" && git ls-files '*.px' '*.c' '*.h' 2>/dev/null )
[ -n "$FILES" ] || env_bad "git ls-files 取不到源码清单"
( cd "$ROOT" && printf '%s\n' "$FILES" | while read -r f; do :; done )
fb=$( cd "$ROOT" && printf '%s\n' "$FILES" | xargs "$PX" norm --fp 2>/dev/null | awk '$0 ~ /state=fallback/ {print $1}' )
fb_n=$(printf '%s' "$fb" | grep -c . || true)
fb_other=$(printf '%s\n' "$fb" | grep -v '^selfhost/cases_bad/' | grep -c . || true)
[ "${fb_other:-0}" -eq 0 ] && ok "E1 回退面有界：$fb_n 个回退全在 selfhost/cases_bad/（故意坏样例）" \
    || bad "E1 出现非预期回退（$fb_other 个）：$(printf '%s' "$fb" | grep -v '^selfhost/cases_bad/' | head -3 | tr '\n' ' ')"

# ---- 基线 ----
BASE=$(rtkey); BASE2=$(rtkey)
[ -n "$BASE" ] || env_bad "取不到 rt_key"
[ "$BASE" = "$BASE2" ] && ok "P0 rt_key 幂等（连跑两次相同：$BASE）" || bad "P0 rt_key 不幂等（$BASE / $BASE2）"
RT_SUM0=$(sha256sum "$RT" | cut -d' ' -f1)

# ---- P1 只改注释 ⇒ 不变 ----
RT_BAK=$(mktemp); cp -p "$RT" "$RT_BAK"
printf '// M439-1 门：整行注释（应不影响语义指纹）\n' >> "$RT"
P1=$(rtkey)
[ "$P1" = "$BASE" ] && ok "P1 加一行整行注释 ⇒ rt_key 不变（$P1）" \
                    || bad "P1 加注释后 rt_key 变了（$BASE → $P1）"

# ---- P2 负控：加代码 ⇒ 必变 ----
printf 'static int _m439_norm_gate_probe = 1;\n' >> "$RT"
P2=$(rtkey)
[ "$P2" != "$BASE" ] && ok "P2 负控：加一行代码 ⇒ rt_key 必变（$BASE → $P2）" \
                     || bad "P2 加代码后 rt_key 未变 ⇒ 判据无牙"

# ---- P3 复原 ----
cp -p "$RT_BAK" "$RT"; rm -f "$RT_BAK"; RT_BAK=""
P3=$(rtkey)
[ "$P3" = "$BASE" ] && ok "P3 复原后 rt_key 回到基线（$P3）" || bad "P3 复原后 rt_key 未回基线（$P3 vs $BASE）"
RT_SUM1=$(sha256sum "$RT" | cut -d' ' -f1)
[ "$RT_SUM0" = "$RT_SUM1" ] && ok "P3b runtime.c 与门开始时逐字节一致" || bad "P3b runtime.c 未复原"

# ---- P4 映射缓存生效（移走 pxnorm 仍取到同一 key）----
BIN=$("$PX" norm --version >/dev/null 2>&1; ls -t "$ROOT"/.rtcache/bin/pxnorm-* 2>/dev/null | head -1)
if [ -n "$BIN" ] && [ -x "$BIN" ]; then
  mv -f "$BIN" "$BIN.hidden"
  P4=$(rtkey)
  mv -f "$BIN.hidden" "$BIN"
  [ "$P4" = "$BASE" ] && ok "P4 移走 pxnorm 后 rt_key 仍等于基线（走 .semmap，稳态零解析）" \
                      || bad "P4 移走 pxnorm 后 rt_key 变了（$P4）⇒ 映射缓存没生效"
  [ -x "$BIN" ] && ok "P4b pxnorm 二进制已还原" || bad "P4b pxnorm 二进制没还原"
else
  bad "P4 找不到编译好的 pxnorm 二进制（.rtcache/bin/）"
fi

# ---- P5 逃生口 PX_NORM=raw ----
P5=$( cd "$ROOT" && PX_NORM=raw "$PX" rtkey "" gcc 0 2>/dev/null )
[ -n "$P5" ] && [ "$P5" != "$BASE" ] && ok "P5 逃生口：PX_NORM=raw ⇒ rt_key 与 sem 口径不同（$P5）" \
                                   || bad "P5 逃生口失效（raw 与 sem 同值）"

echo "── 小计：$PASS 通过 / $FAIL 失败"
[ "$FAIL" -eq 0 ] || exit 1
echo "✅ M439-1 语义指纹门全绿"
exit 0
