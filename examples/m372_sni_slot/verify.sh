#!/usr/bin/env bash
# ============================================================
# examples/m372_sni_slot/verify.sh —— SNI 证书槽容量门（缺陷 657 · M372）
# ------------------------------------------------------------
# 为什么有这道门（2026-11-01 17:47 晨曦经总线转办，我挂了 26h54m 未处理）：
#   晨曦给 id.xiusoft.cn 装第 17 张证书 ⇒ register_tls 报
#   「SNI 证书数量超出上限 16」⇒ **整机 443 全挂**（注册失败发生在启动路径）。
#   官方包容量 16 ⇒ 任何人装到第 17 张就是整机拒服。
#
# 修法（M372）：容量 16 → 默认 256（`#ifndef PX_MAX_SNI_CERTS` 可编译期覆盖）
#   + 新增内建 `px_sni_capacity()`（容量可读 ⇒ 上层能在启动前拦住）
#   + 溢出错误带「已用 N / 容量 M + hostname」（原来只有一句「上限 16」，不可诊断）。
#   ⚠ **不做** realloc 动态扩容：px_sni_cb 把 &g_sni_certs[slot].cert 交给 SSL 上下文，
#     数组搬迁 = UAF（缺陷 414 / 655 同族）。容量必须编译期固定。
#
# 判据：
#   P1 容量可读：px_sni_capacity() 三轨（解释/VM/C）都返回 256
#   P2 **事故复现**：17 个不同 hostname 注册全部成功（旧版第 17 个必失败）
#   P3 边界：256 个不同 hostname 注册全部成功（满槽不误报）
#   P4 槽复用未破坏：同一 hostname 注册 300 次恒定成功（不是"每张新开槽"）
#   P5 溢出契约：第 257 个 ⇒ rc≠0 且消息含「已用 256 / 容量 256」+ hostname
#   P6 源码契约：宏在 #ifndef 内 · g_sni_certs 无 realloc 搬迁 · px_sni_cb 仍取地址
#   P7 覆盖度自证
#
# 负控：
#   N1 把 P5 的期望改成「不报错」⇒ 必红（错误格有鉴别力）
#   N2 若容量宏被改回 16（副本 sed 改 + 仅看 P1 口径）：P1 会红 —— 用「读实现真值」
#      的自证代替重编：P1 的红只能由宏变回 16 触发，故 P2(17 张) 与 P1 互为负控。
#
# 用法：verify.sh [--neg-skip]    退出码：0=全绿 · 1=有红
# ⚠ 前置 S4 重烘（tools/px 必须含本次 runtime 改动）
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m372_sni_slot.$$"
NEG_SKIP=0; [ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK"

[ -x "$PX" ] || { echo "❌ 找不到 $PX（先 selfhost/rebake_bin.sh）"; exit 1; }
command -v openssl >/dev/null || { echo "❌ 缺 openssl"; exit 1; }

echo "═══ 缺陷 657 · SNI 证书槽容量门 ═══"

# ---------- 证书夹具（一份 ECC 自签，快） ----------
openssl req -x509 -newkey ec -pkeyopt ec_paramgen_curve:prime256v1 -keyout "$WORK/k.pem" \
  -out "$WORK/c.pem" -days 1 -nodes -subj /CN=sni-slot.test >/dev/null 2>&1
[ -s "$WORK/c.pem" ] && [ -s "$WORK/k.pem" ] || { echo "❌ 证书夹具生成失败"; exit 1; }
note "夹具：1 张 ECC 自签证书（复用给 N 个 hostname）"

# ---------- P1 容量可读（三轨） ----------
cat > "$WORK/cap.px" <<'EOF'
def main():
    print(px_sni_capacity())
EOF
"$PX" run "$WORK/cap.px" > "$WORK/cap.interp" 2> "$WORK/cap.err"; r1=$?
rm -rf "$WORK/build"; ( cd "$WORK" && "$PX" build "$WORK/cap.px" > "$WORK/cap.vmlog" 2>&1 ); rb=$?
if [ $rb -eq 0 ] && [ -x "$WORK/build/cap" ]; then "$WORK/build/cap" > "$WORK/cap.vm" 2>&1; else : > "$WORK/cap.vm"; fi
rm -rf "$WORK/build"; ( cd "$WORK" && "$PX" build --c "$WORK/cap.px" > "$WORK/cap.clog" 2>&1 ); rc2=$?
if [ $rc2 -eq 0 ] && [ -x "$WORK/build/cap" ]; then "$WORK/build/cap" > "$WORK/cap.c" 2>&1; else : > "$WORK/cap.c"; fi
rm -rf "$WORK/build"

CAP="$(head -1 "$WORK/cap.interp" 2>/dev/null | tr -d '[:space:]')"
if [ "$r1" = 0 ] && [ "$CAP" = "256" ]; then ok "P1 解释轨 px_sni_capacity() = 256"
else bad "P1 解释轨容量不是 256（rc=$r1，得 '$CAP'）$(head -1 "$WORK/cap.err")"; fi
if [ "$(head -1 "$WORK/cap.vm" | tr -d '[:space:]')" = "256" ]; then ok "P1 VM 轨 = 256"
else bad "P1 VM 轨不是 256：$(head -3 "$WORK/cap.vmlog" | tr '\n' ' ')"; fi
if [ "$(head -1 "$WORK/cap.c" | tr -d '[:space:]')" = "256" ]; then ok "P1 C 轨 = 256"
else bad "P1 C 轨不是 256：$(head -3 "$WORK/cap.clog" | tr '\n' ' ')"; fi

# ---------- 造「注册 N 个不同 hostname」的程序 ----------
gen_px() {   # $1=N $2=输出
  {
    printf 'def main():\n'
    printf '    let c = "%s"\n' "$WORK/c.pem"
    printf '    let k = "%s"\n' "$WORK/k.pem"
    local i
    for i in $(seq 1 "$1"); do
      printf '    assert(tls_server(c, k, "h%d.sni-slot.test"))\n' "$i"
    done
    printf '    print("registered %s")\n' "$1"
  } > "$2"
}

# ---------- P2 事故复现：17 张 ----------
gen_px 17 "$WORK/p17.px"
"$PX" run "$WORK/p17.px" > "$WORK/p17.out" 2> "$WORK/p17.err"; rp2=$?
if [ $rp2 -eq 0 ] && grep -q "registered 17" "$WORK/p17.out"; then
  ok "P2 事故复现：17 个 hostname 全部注册成功（旧版第 17 个必报「上限 16」）"
else
  bad "P2 第 17 张证书仍失败（rc=$rp2）：$(head -2 "$WORK/p17.err" | tr '\n' ' ')"
fi

# ---------- P3 边界：256 张 ----------
gen_px 256 "$WORK/p256.px"
"$PX" run "$WORK/p256.px" > "$WORK/p256.out" 2> "$WORK/p256.err"; rp3=$?
if [ $rp3 -eq 0 ] && grep -q "registered 256" "$WORK/p256.out"; then
  ok "P3 边界：256 个 hostname 全部注册成功（满槽不误报）"
else
  bad "P3 满槽误报（rc=$rp3）：$(head -2 "$WORK/p256.err" | tr '\n' ' ')"
fi

# ---------- P4 槽复用未破坏：同名 300 次 ----------
cat > "$WORK/same.px" <<EOF
def main():
    let c = "$WORK/c.pem"
    let k = "$WORK/k.pem"
    let i = 0
    while i < 300:
        assert(tls_server(c, k, "same.sni-slot.test"))
        i = i + 1
    print("reused 300")
EOF
"$PX" run "$WORK/same.px" > "$WORK/same.out" 2> "$WORK/same.err"; rp4=$?
if [ $rp4 -eq 0 ] && grep -q "reused 300" "$WORK/same.out"; then
  ok "P4 同名覆盖仍在（300 次注册不占新槽、不误报）"
else
  bad "P4 槽复用被破坏（rc=$rp4）：$(head -2 "$WORK/same.err" | tr '\n' ' ')"
fi

# ---------- P5 溢出契约：第 257 个 ----------
gen_px 257 "$WORK/p257.px"
"$PX" run "$WORK/p257.px" > "$WORK/p257.out" 2> "$WORK/p257.err"; rp5=$?
MSG="$(head -1 "$WORK/p257.err")"
if [ $rp5 -ne 0 ] && grep -q "已用 256 / 容量 256" "$WORK/p257.err" && grep -q "h257.sni-slot.test" "$WORK/p257.err"; then
  ok "P5 第 257 个按要求拒绝，消息含「已用 256 / 容量 256」+ hostname"
else
  bad "P5 溢出契约不符（rc=$rp5）：$MSG"
fi

# ---------- P6 源码契约 ----------
SRC="$ROOT/runtime/runtime.c"
c6=0
grep -q '#ifndef PX_MAX_SNI_CERTS' "$SRC" && c6=$((c6+1))
grep -q '#define PX_MAX_SNI_CERTS 256' "$SRC" && c6=$((c6+1))
# 数组必须是静态定长（不得变成指针/realloc 形态）
grep -qE 'static PxSniCert g_sni_certs\[PX_MAX_SNI_CERTS\]' "$SRC" && c6=$((c6+1))
# 不得对 g_sni_certs 做 realloc（搬迁 = UAF）
if grep -qE 'realloc\s*\(\s*g_sni_certs' "$SRC"; then c6=$((c6-10)); fi
[ "$c6" -ge 3 ] && ok "P6 源码契约：宏可覆盖 · 数组静态定长 · 无 realloc 搬迁" \
               || bad "P6 源码契约不符（命中 $c6/3）"

# ---------- P7 覆盖度自证 ----------
cov=0
grep -q 'assert(tls_server(c, k, "h17.sni-slot.test"))' "$WORK/p17.px" && cov=$((cov+1))
grep -q 'assert(tls_server(c, k, "h256.sni-slot.test"))' "$WORK/p256.px" && cov=$((cov+1))
grep -q 'assert(tls_server(c, k, "h257.sni-slot.test"))' "$WORK/p257.px" && cov=$((cov+1))
[ "$cov" -ge 3 ] && ok "P7 覆盖度自证：17/256/257 三个判据点都真在用例里" || bad "P7 用例被改空（$cov/3）"

# ---------- 负控 ----------
if [ "$NEG_SKIP" = 1 ]; then
  note "负控跳过（--neg-skip）"
else
  echo "--- 负控 ---"
  # N1：把 P5 的期望反转 ⇒ 必须红（前提自证：证明 257 确实报错了）
  if [ $rp5 -ne 0 ]; then
    note "N1 前提自证：257 号 rc=$rp5（真报错）"
    ok "N1 若期望「不报错」则必然红 ⇒ 错误格有鉴别力"
  else
    bad "N1 前提不成立：257 号没报错，P5 判据无意义"
  fi
  # N2：容量宏若仍是 16，P1 必红 —— 用「P1 读数 == 源码宏值」交叉自证
  MACRO="$(grep -oE '#define PX_MAX_SNI_CERTS [0-9]+' "$SRC" | grep -oE '[0-9]+$' | head -1)"
  if [ -n "$MACRO" ] && [ "$CAP" = "$MACRO" ]; then
    ok "N2 自证：P1 读数($CAP) == 源码宏($MACRO) ⇒ 容量不是写死在别处"
  else
    bad "N2 读数与源码宏不一致（读 $CAP / 宏 ${MACRO:-?}）"
  fi
fi

echo
echo "PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ]
