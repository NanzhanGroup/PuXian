#!/usr/bin/env bash
# ============================================================
# check_m448_smtp954.sh —— 缺陷 954（M448）：std.smtp 接入 STARTTLS / implicit TLS
# ------------------------------------------------------------
# 为什么有这条门（2026-11-27 东月核实）：
#   dce1294a 改了 stdlib/smtp.px + registry/smtp/0.1.0，并写了判据执行体
#   selfhost/assets/smtp954_{client.px,server.py,run.sh}（18 断言），但
#   **没有把它变成门** —— gates.registry.sh / ci.yml / gate_time_baseline.tsv 命中数均为 0。
#   ⇒ 谁再改 smtp.px 都不会被挡。这正是「改了包没跟上配套」的同族（缺陷 963 那批）。
#
# 判据分层：
#   P   前提：tools/px · python3 · openssl · stdlib/smtp.px 缺 ⇒ rc=3（ENV，不算红也不算绿）
#   C1  E1 判据体全绿（smtp954_run.sh：S1 明文/S2 STARTTLS/S3 implicit/S4 不降级/S5 未通告/S6 非法值）
#   C2  静态：断言数 ≥ 18 且四个关键断言在场（防空跑恒绿 / 防有人把判据删掉一半）
#   C3  关键语义锚：smtp.px 里 tls 分支必须真的存在（"starttls"/"implicit" 字面量）
#   N1  负控：临时副本里把 client 的 starttls 降级成 plain ⇒ S2 必红（证明 C1 有牙）
#   N1b 恢复后重新绿（自证负控扰动真的只作用于副本）
#
# 退出：0 全绿 / 1 有红 / 3 ENV（缺依赖，跳过）
# 纪律：**绝不改真树** —— 负控只在 /tmp 副本上做（缺陷 961 的教训）
# ============================================================
set -uo pipefail
SELF_DIR=$(cd "$(dirname "$0")" && pwd)
REPO=${REPO:-$(cd "$SELF_DIR/.." && pwd)}
RUN="$SELF_DIR/assets/smtp954_run.sh"
CLI="$SELF_DIR/assets/smtp954_client.px"
SMTP="$REPO/stdlib/smtp.px"
OUT=${M448_SMTP954_OUT:-/data/gatevm/m448-954-gate}
pass=0; fail=0
ck() { if [ "$2" = "$3" ]; then echo "✓ $1"; pass=$((pass+1)); else echo "✗ $1（want [$3] got [$2]）"; fail=$((fail+1)); fi; }
say() { echo "[m448_smtp954] $*"; }

# ── P 前提 ──
for c in python3 openssl timeout; do
  command -v "$c" >/dev/null 2>&1 || { say "ENV：缺 $c ⇒ rc=3（跳过）"; exit 3; }
done
[ -x "$REPO/tools/px" ] || { say "ENV：缺 $REPO/tools/px ⇒ rc=3"; exit 3; }
[ -f "$SMTP" ]          || { say "ENV：缺 $SMTP ⇒ rc=3"; exit 3; }
[ -f "$RUN" ]           || { say "ENV：缺 $RUN ⇒ rc=3"; exit 3; }
[ -f "$CLI" ]           || { say "ENV：缺 $CLI ⇒ rc=3"; exit 3; }

# ── C2 静态：判据不许被删掉一半、不许空跑 ──
n_ck=$(grep -c '^ck ' "$RUN" 2>/dev/null || echo 0)
ck "C2 判据体断言数 ≥18（防被删剩几条仍绿）" "$([ "${n_ck:-0}" -ge 18 ] && echo yes || echo no)" yes
for anchor in 'S2c 升级后重新 EHLO' 'S4b 服务端没收到明文 MAIL FROM' 'S5b 报错点名' 'S1b 服务端记录 tls=false'; do
  ck "C2 关键断言在场：${anchor:0:12}" "$(grep -c "$anchor" "$RUN" 2>/dev/null || echo 0)" 1
done

# ── C3 语义锚：tls 分支真的在 smtp.px 里 ──
ck "C3 smtp.px 有 starttls 分支" "$([ "$(grep -c '"starttls"' "$SMTP" 2>/dev/null || echo 0)" -ge 1 ] && echo yes || echo no)" yes
ck "C3b smtp.px 有 implicit 分支" "$([ "$(grep -c '"implicit"' "$SMTP" 2>/dev/null || echo 0)" -ge 1 ] && echo yes || echo no)" yes

# ── C1 E1 判据体 ──
mkdir -p "$OUT"
rm -rf "$OUT/main"; mkdir -p "$OUT/main"
SMTP954_OUT="$OUT/main" bash "$RUN" "$REPO" > "$OUT/main.log" 2>&1
rc_main=$?
tail -3 "$OUT/main.log" | sed 's/^/    /'
ck "C1 E1 判据体全绿（18 断言）rc=0" "$rc_main" 0
ck "C1b 汇总行 pass/fail 有值" "$(grep -c 'M448_SMTP_TLS pass=' "$OUT/main.log" 2>/dev/null || echo 0)" 1

# ── N1 负控：临时副本里把 starttls 降级成 plain ⇒ S2 必红 ──
NEG="$OUT/neg"
rm -rf "$NEG"; mkdir -p "$NEG/assets"
cp "$RUN" "$NEG/assets/" 2>/dev/null
cp "$SELF_DIR/assets/smtp954_server.py" "$NEG/assets/" 2>/dev/null
sed 's/if mode == "starttls":/if mode == "__starttls_disabled__":/' "$CLI" > "$NEG/assets/smtp954_client.px"
neg_changed=$(diff -q "$CLI" "$NEG/assets/smtp954_client.px" >/dev/null 2>&1 && echo no || echo yes)
ck "N1 前提自证：副本真的被改了" "$neg_changed" yes
SMTP954_OUT="$NEG/out" bash "$NEG/assets/smtp954_run.sh" "$REPO" > "$OUT/neg.log" 2>&1
rc_neg=$?
ck "N1 负控：TLS 升级被阉 ⇒ 判据体必红（rc≠0）" "$([ "$rc_neg" -ne 0 ] && echo yes || echo no)" yes
ck "N1b 负控红的正是 S2 场景" "$([ "$(grep -c '❌ S2' "$OUT/neg.log" 2>/dev/null || echo 0)" -ge 1 ] && echo yes || echo no)" yes

# ── N1c 真树没被碰（缺陷 961 的纪律）──
ck "N1c 真树 smtp954_client.px 未被改动" "$(cd "$REPO" && git status --porcelain selfhost/assets/smtp954_client.px 2>/dev/null | wc -l)" 0

echo "----------------------------------------"
echo "M448_SMTP954-OK pass=$pass fail=$fail"
[ "$fail" = 0 ] || exit 1
exit 0
