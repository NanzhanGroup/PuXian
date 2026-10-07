#!/usr/bin/env bash
# M283 门（第 161 轮）：判据的「第 0 维：前提」—— selfhost/check_gate_premise.sh
#
# 立论（三次同族事故，全都不在产品代码）：
#   · M280「六轮读数全 0」判据全过、结论全错（残留进程占端口 ⇒ 服务没起来）
#   · M282「提交前绿、提交后红」（缺陷 491：扫描面漏掉未跟踪文件）
#   · 缺陷 487「干跑通过 ≠ 同步能过」（tar | grep -q + pipefail 恒假）
#   ⇒ 共同形状：**判据自己的前提没有被验证过**。
#
# 本门 7 层：[1] 守卫自证 · [2] 真仓 rc=0 + 规模锚点 · [3] 4 类反例逐类指名 ·
#            [4] 登记过期判据 · [5] 负控 3 道（各自独立判红） · [6] 覆盖边界 · [7] 静态断言
#
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"

G=../../selfhost/check_gate_premise.sh
ALLOW=../../selfhost/premise_allow.tsv
PASS=0; FAIL=0
ok(){ echo "  ✅ $1"; PASS=$((PASS+1)); }
bad(){ echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk(){ if [ "$2" = "$3" ]; then ok "$1"; else bad "$1（actual=[$2] want=[$3]）"; fi; }
jget(){ sed -nE "s/.*\"$1\":([0-9]+).*/\1/p" <<<"$2"; }

W="$(mktemp -d)"
BAK="$W/guard.bak"
cp -a "$G" "$BAK"
restore_all(){ cp -a "$BAK" "$G"; }
# ⚠️ M269/M270 纪律：**每道负控前独立还原**（否则后一道从「已污染」的源出发 ⇒ 判据失效）
neg_start(){ restore_all; }
trap 'restore_all; rm -rf "$W"' EXIT

echo "== [1] 守卫自证（内置夹具 · 10 断言）=="
out="$(bash "$G" --self-test 2>&1)"; rc=$?
chk "自证 rc=0" "$rc" "0"
chk "自证 10 通过 / 0 失败" "$(grep -c '自证：通过 10 / 失败 0' <<<"$out")" "1"
chk "自证含反向判据（登记后 rc=0）" "$(grep -c '⑤ 全部登记后 rc=0' <<<"$out")" "1"
chk "自证含判据自伤（清空表 ⇒ rc=1）" "$(grep -c '⑥ 判据自伤' <<<"$out")" "1"

echo "== [2] 真仓扫描：rc=0 + 规模锚点 =="
j="$(bash "$G" --json 2>&1)"; rc=$?
chk "真仓 rc=0（零违例 · 零过期）" "$rc" "0"
n="$(jget gates "$j")"
[ "${n:-0}" -ge 190 ] && ok "规模锚点：门 $n ≥ 190" || bad "规模锚点失败：门 $n"
sh="$(jget shared "$j")"
[ "${sh:-0}" -ge 8 ] && ok "规模锚点：P1 共用端口 $sh ≥ 8" || bad "规模锚点失败：P1 $sh"
chk "P2 弱就绪 = 0（m137 已补真连接探测）" "$(jget p2 "$j")" "0"
chk "P3 禁形 = 0（3 处真风险已改正，零豁免）" "$(jget p3 "$j")" "0"
chk "P4 前提 = 2（issue28_b1 的变量式创建，已登记）" "$(jget p4 "$j")" "2"

echo "== [3] 判据有牙：4 类反例，守卫必须逐类指名 =="
S="$W/sub"; mkdir -p "$S/examples"/{g_port_a,g_port_b,g_ready,g_prem} "$S/selfhost"
git -C "$S" init -q 2>/dev/null || true
printf 'PORT=39001\n'                 > "$S/examples/g_port_a/verify.sh"
printf '127.0.0.1:39001\n'            > "$S/examples/g_port_b/verify.sh"
# ⚠️ 夹具内容在**运行时**才是 `grep -q READY`；源码里写成 `RE%sADY` ⇒ 本门自身不被 P2 判（夹具与判据隔离）
printf '#!/bin/bash\ngrep -q RE%sADY srv.log\n' "" > "$S/examples/g_ready/verify.sh"
mkdir -p "$S/examples/g_pipe"
printf '#!/bin/bash\nset -o pipefail\nreadelf -d /bin/sh | grep -q NEEDED\n' > "$S/examples/g_pipe/verify.sh"
printf '#!/bin/bash\nif [ -f /tmp/m283_prem_absent/x.txt ]; then cat /tmp/m283_prem_absent/x.txt; fi\n' \
  > "$S/examples/g_prem/verify.sh"
: > "$S/selfhost/premise_allow.tsv"
o3="$(bash "$G" --root "$S" 2>&1)"; rc3=$?
chk "[3] 子仓库未登记 ⇒ rc=1" "$rc3" "1"
chk "[3a] P1 指名端口 39001 与两个门" "$(grep -c '^P1 .*39001.*g_port_a.*g_port_b' <<<"$o3")" "1"
chk "[3b] P2 指名 g_ready"            "$(grep -c '^P2 .*g_ready' <<<"$o3")" "1"
chk "[3c] P4 指名 g_prem 与缺失路径"   "$(grep -c '^P4 .*g_prem.*m283_prem_absent' <<<"$o3")" "1"
chk "[3] 违例数 = 4（1+1+1+1）"        "$(grep -cE '^(P1|P2|P3|P4) ' <<<"$o3")" "4"

echo "== [4] 登记过期判据（表里有、实测没有 ⇒ rc=3）=="
printf 'P1\t39999\tstale\t2026-10-07\tM283\n' > "$S/selfhost/premise_allow.tsv"
o4="$(bash "$G" --root "$S" 2>&1)"; rc4=$?
chk "[4] 过期登记 ⇒ rc=3" "$rc4" "3"
chk "[4] 消息含「登记过期」且指名 39999" "$(grep -c '登记过期.*39999' <<<"$o4")" "1"
: > "$S/selfhost/premise_allow.tsv"

echo "== [5] 负控 3 道（各自独立判红）=="
if [ "${NEG_SKIP:-0}" = 1 ]; then
  echo "  ⊘ 负控已跳过（NEG_SKIP=1 —— CI 用；本地全量门跑全量档）"
else
# NC-A：忠实抽掉 P1 判据 ⇒ 子仓库违例 4 → 3（P1 不再指名）
neg_start
sed -i 's|if \[ "\$AUDIT" = 0 \] && ! allow_has P1 "\$p"; then|if false; then|' "$G"
oA="$(bash "$G" --root "$S" 2>&1)"
chk "NC-A 抽掉 P1 判据 ⇒ 违例 4→3" "$(grep -cE '^(P1|P2|P3|P4) ' <<<"$oA")" "3"
chk "NC-A P1 不再指名"              "$(grep -c '^P1 ' <<<"$oA")" "0"
# NC-B：抽掉 P4 的命名空间过滤 ⇒ 真仓 P4 数上升（门自己的路径不再被过滤）
neg_start
sed -i 's|if \[ -n "\$gnum" \] && grep -qF "\$gnum" <<<"\$p"; then continue; fi|:|' "$G"
sed -i 's|if grep -qF "\$gid" <<<"\$p"; then continue; fi|:|' "$G"
jB="$(bash "$G" --json 2>&1)"
p4B="$(jget p4 "$jB")"
[ "${p4B:-0}" -gt 2 ] && ok "NC-B 抽掉命名空间过滤 ⇒ P4 由 2 升到 $p4B" || bad "NC-B 未生效：P4=$p4B"
# NC-C：判据自伤 —— 抽掉 P1 判据 **且** 让消息输出静默 ⇒ NC-A 的「数消息行」判据必须失效
neg_start
sed -i 's|if \[ "\$AUDIT" = 0 \] && ! allow_has P1 "\$p"; then|if false; then|' "$G"
sed -i 's|^emit() { MSGS+=("$1"); }$|emit() { :; }|' "$G"
oC="$(bash "$G" --root "$S" 2>&1)"
chk "NC-C 判据自伤 ⇒ NC-A 的判据失效（读到 0 ≠ 3）" "$(grep -cE '^(P1|P2|P3|P4) ' <<<"$oC")" "0"
restore_all
fi

echo "== [6] 覆盖边界（如实登记，不判红）=="
cat <<'EDGE'
  · P1 只判**字面量**端口；`$PORT` 变量 / 环境注入 / CI 侧端口**不在面内**（envelope 由 m116 的 PID 锁覆盖）
  · P2 只判「脚本用 grep 阻塞等待就绪字样（RE%sADY 等）」形态；等就绪**文件**、硬等 sleep、等 stdout 不属本判据
    （此处刻意不写出字面关键词 —— 夹具与判据隔离：本门自身不得被自己的 P2 判据判红）
  · P3 只判**危险清单**左侧（cat/tar/readelf/nm/strings/objdump/ldd/dnf/rpm/curl/ssh/git/find/du/sort/程序和 ./）；
    `echo "$VAR"` / `printf '%s' "$VAR"` 理论上也可能超管道缓冲（64KB）但内容不可静态判定 ⇒ 不判
  · P4 已知盲区：**变量形式的创建**（`> /tmp/x_${LABEL}.press`）⇒ 会误报为外部前提（issue28_b1 已登记）
EDGE
ok "[6] 覆盖边界已打印（4 条）"

echo "== [7] 静态断言 =="
[ -x "$G" ] && ok "[7a] 守卫可执行" || bad "[7a] 守卫不可执行"
[ -f "$ALLOW" ] && ok "[7b] 允许表存在" || bad "[7b] 允许表缺失"
chk "[7c] 允许表 P3 段为空（发现即修，零豁免）" "$(awk -F'\t' '!/^#/ && NF>=5 && $1=="P3"' "$ALLOW" | wc -l)" "0"
chk "[7d] 允许表每条都有理由+日期+依据" \
    "$(awk -F'\t' '!/^#/ && NF>0 && (NF<5 || $3=="" || $4=="" || $5=="")' "$ALLOW" | wc -l)" "0"
chk "[7e] 守卫头注声明四类判据" "$(grep -cE '^#   P[1-4] ' "$G")" "4"

echo "────────────────────────────"
echo "M283 门：通过 $PASS / 失败 $FAIL"
[ "$FAIL" = 0 ] && echo "M283-VERIFY-OK" || echo "M283-VERIFY-FAIL"
exit "$([ "$FAIL" = 0 ] && echo 0 || echo 1)"
