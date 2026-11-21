#!/usr/bin/env bash
# ============================================================
# selfhost/check_gate_portscan.sh —— 「门跑完不许留监听」的判据（M430 · 缺陷 879）
# ------------------------------------------------------------
# 病史（2026-11-21 19:5x 实测）：
#   造一个「起监听就退出」的假门 → 修前 run_gates.sh 判 `✅ leaky_demo`、TSV 记 OK，
#   而 ss 里 127.0.0.1:38769 仍在 LISTEN、PPID=1。300+ 门里只有 M429 给自己那扇
#   加了收尾判据 ⇒ 其余全是盲区。本月四起残留（15 个 build/t 挂 7.5h / 6 个 18902
#   七天 / 18096 / 58717 十二天）没有一起是本门自己发现的。
#
# 判据：
#   S1 形状：run() 里「残留扫描」必须在判 OK **之前**；marker 必须**只在这一条命令**
#            上做前缀（全局 export 会让所有门共用一个 marker ⇒ 归属失效）；
#            失败分支必须回收残留（不重复记账）。
#   S2 形状：判绿路径**不许**用快照差集（分片 8 片并发 ⇒ 必然误归因），
#            差集只能作为人工诊断（snapshot --diff）。
#   P2 前提自证：孤儿（PPID=1）进程的 /proc/<pid>/environ 里**真的**还能读到 marker
#            —— 这是整个归属机制的地基，不成立则 P3/N1 不许判绿。
#   P3 核心：按 marker 必须认出孤儿监听（rc=1 且点名 pid）。
#   P4 不得误伤：别的 marker 扫描不许报它、不许杀它。
#   N1 负控（端到端）：临时注册源里一个**故意泄漏**的假门 → 运行器必须判红，
#            点名 portleak + 端口，回收后端口必须真空。
#   P1 正控（防过度修复）：「起服务→自己收走」的干净假门 → 必须判 OK。
#   P5 豁免账：仓内表格式合法（理由≥8 / 到期日 / 上限）；命中豁免的门不判红。
#   P6 负控自证：仓外副本把判据摘除 ⇒ 同一泄漏门必须变绿（且残留真的留下）。
#   G1 环境守卫：没有 ss ⇒ rc=3（绝不降级成绿）。
#   G2 零污染：仓库 git status 行数前后一致。
#   G3 收尾：本门自己不许留监听。
#
# 用法：check_gate_portscan.sh              # 全跑（约 40~90s）
#       check_gate_portscan.sh --self-test  # 只跑判据夹具（离线 <2s）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCAN="$ROOT/selfhost/gate_portscan.sh"
RUNNER="$ROOT/selfhost/run_gates.sh"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m430_portscan.XXXXXX")"
PASS=0; FAIL=0
ok(){ echo "  PASS $1"; PASS=$((PASS+1)); }
bad(){ echo "  FAIL $1"; FAIL=$((FAIL+1)); }

KIDS=()
cleanup(){ local p; for p in "${KIDS[@]:-}"; do [ -n "$p" ] && kill -KILL "$p" 2>/dev/null; done; rm -rf "$TMP"; }
trap cleanup EXIT

pick_port(){ python3 - <<'EOF'
import socket
s=socket.socket(); s.bind(("127.0.0.1",0)); print(s.getsockname()[1]); s.close()
EOF
}
listen_has(){ ss -H -lntup 2>/dev/null | grep -q ":$1 "; }
pid_of(){ ss -H -lntup 2>/dev/null | grep "127.0.0.1:$1 " | grep -o 'pid=[0-9]*' | head -1 | cut -d= -f2; }

# ── 自测夹具（离线）────────────────────────────────────────
self_test() {
    local P=0 F=0
    s_ok(){ echo "  PASS $1"; P=$((P+1)); }
    s_no(){ echo "  FAIL $1"; F=$((F+1)); }
    bash "$SCAN" --self-test > "$TMP/st.log" 2>&1 \
        && s_ok "0 机制自测全过" || { s_no "0 机制自测失败"; sed 's/^/     /' "$TMP/st.log"; }
    grep -q 'PASS 13 / FAIL 0' "$TMP/st.log" && s_ok "0b 自测条数锚点 = 13" \
        || s_no "0b 自测条数变了（判据被删？需同步改本锚点）"
    echo "  --- 自测：PASS $P / FAIL $F ---"
    [ "$F" = 0 ] || exit 2
}
[ "${1:-}" = "--self-test" ] && { self_test; exit 0; }

# ── G1 环境守卫 ────────────────────────────────────────────
command -v ss >/dev/null 2>&1 || { echo "rc=3 环境不成立：没有 ss"; exit 3; }
command -v python3 >/dev/null 2>&1 || { echo "rc=3 环境不成立：没有 python3"; exit 3; }
[ -f "$SCAN" ] && [ -f "$RUNNER" ] || { echo "rc=3 环境不成立：缺 $SCAN 或 $RUNNER"; exit 3; }

echo "=== M430 端口残留扫描：门跑完不许留监听 ==="
GIT0="$(git -C "$ROOT" status --porcelain 2>/dev/null | wc -l)"

# ── S1/S2 形状 ─────────────────────────────────────────────
L_SCAN=$(grep -n '_gate_portleak "\$name" "\$mark" "\$el"' "$RUNNER" | head -1 | cut -d: -f1)
L_OK=$(grep -n '_gate_adjudicate_ok_rc "\$name" "\$el"' "$RUNNER" | head -1 | cut -d: -f1)
L_CLEAN=$(grep -n '_gate_portclean "\$name" "\$mark"' "$RUNNER" | head -1 | cut -d: -f1)
if [ -n "$L_SCAN" ] && [ -n "$L_OK" ] && [ "$L_SCAN" -lt "$L_OK" ]; then
    ok "S1a 扫描在判 OK 之前（行 $L_SCAN < $L_OK —— 先记绿就再也改不回来）"
else
    bad "S1a 扫描不在判 OK 之前（scan=${L_SCAN:-缺} ok=${L_OK:-缺}）"
fi
grep -q 'PUXIAN_GATE_MARK="\$mark" timeout' "$RUNNER" \
    && ok "S1b marker 只在该条命令上做前缀（不是全局 export）" \
    || bad "S1b marker 没按门前缀：全局 export 会让所有门共用 marker ⇒ 归属全失效"
[ -n "$L_CLEAN" ] && ok "S1c 失败分支回收残留（行 $L_CLEAN）" || bad "S1c 失败门不回收，残留会攒到端口耗尽"
if awk '/^cmd_scan\(\)/{f=1} f&&/^}/{exit} f' "$SCAN" | grep -qE '\bcomm\b|--diff|snapshot'; then
    bad "S2 判绿路径里出现快照差集（分片 8 片并发 ⇒ 必然误归因）"
else
    ok "S2 判绿只靠 marker 归属；差集只留在 snapshot 诊断通道"
fi
grep -q '不参与判绿' "$SCAN" && ok "S2b 差集的定位在源码里写明（人工诊断，不判绿）" || bad "S2b 没写明差集不参与判绿"

# ── P2 前提自证：孤儿进程的 environ 里 marker 还在 ───────────
PP="$(pick_port)"; MARK="m430probe-$$-$RANDOM"
M430_PORT="$PP" PUXIAN_GATE_MARK="$MARK" bash -c \
  'setsid python3 -c "import os,time,socket
s=socket.socket(); s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
s.bind((\"127.0.0.1\",int(os.environ[\"M430_PORT\"]))); s.listen(4)
time.sleep(180)" >/dev/null 2>&1 &'
sleep 1
ORPH="$(pid_of "$PP")"
if [ -n "$ORPH" ]; then
    KIDS+=("$ORPH")
    PPN="$(awk '{print $4}' "/proc/$ORPH/stat" 2>/dev/null)"
    [ "$PPN" = 1 ] && ok "P2a 泄漏进程已成孤儿（PPID=1，pid=$ORPH）" \
                   || bad "P2a 没孤儿化（PPID=$PPN）—— 场景不代表真实残留"
    if tr '\0' '\n' < "/proc/$ORPH/environ" 2>/dev/null | grep -qxF "PUXIAN_GATE_MARK=$MARK"; then
        ok "P2b 前提自证：孤儿 environ 里仍能读到 marker（归属机制的地基）"
    else
        bad "P2b 孤儿 environ 读不到 marker ⇒ P3/N1 的归属结论不成立"
    fi
    # ── P3 核心 / P4 不得误伤 ──
    out="$("$SCAN" scan --mark "$MARK" --gate m430_probe --grace 1 2>&1)"; rc=$?
    if [ "$rc" = 1 ] && printf '%s' "$out" | grep -q "pid=$ORPH"; then
        ok "P3 按 marker 认出孤儿监听（rc=1 且点名 pid=$ORPH）"
    else
        bad "P3 没认出孤儿（rc=$rc）：$(printf '%s' "$out" | head -2 | tr '\n' ' ')"
    fi
    out2="$("$SCAN" scan --mark "another-marker-$$" --gate m430_probe --grace 1 2>&1)"; rc2=$?
    if [ "$rc2" = 0 ] && kill -0 "$ORPH" 2>/dev/null; then
        ok "P4 别的 marker 扫描：不报、也不杀别人的监听"
    else
        bad "P4 误伤/误报（rc=$rc2）：$(printf '%s' "$out2" | head -2 | tr '\n' ' ')"
    fi
    kill -KILL "$ORPH" 2>/dev/null
else
    bad "P2 前提不成立：探测监听没起来（P3/N1 不许当绿）"
fi

# ── N1/P1/P6：端到端（真跑 run_gates.sh）─────────────────────
PORT_LEAK="$(pick_port)"; PORT_OK="$(pick_port)"
mk_reg(){   # $1=文件 $2=泄漏门名 $3=干净门名
    cat > "$1" <<EOF
step "M430 端到端"
run $2 bash -c 'setsid python3 -c "import socket,time;s=socket.socket();s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1);s.bind((\"127.0.0.1\",$PORT_LEAK));s.listen(4);time.sleep(300)" >/dev/null 2>&1 & sleep 1; exit 0'
run $3 bash -c 'python3 -c "import socket,time;s=socket.socket();s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1);s.bind((\"127.0.0.1\",$PORT_OK));s.listen(4);time.sleep(1)" & P=\$!; sleep 1; kill \$P; wait \$P 2>/dev/null; exit 0'
EOF
}
run_nested(){   # $1=runner $2=注册源 $3=日志 $4=tsv
    GATE_REGISTRY="$2" GATE_TSV="$4" \
    PX_GATE_PIDLOCK="$TMP/pl.$$.$RANDOM" PX_GATE_LOCK="$TMP/gl.$$.$RANDOM" \
    GATE_PORTSCAN_GRACE=1 \
        bash "$1" --allow-dirty > "$3" 2>&1
    return $?
}
mk_reg "$TMP/reg.sh" m430_probe_leaky m430_probe_clean
run_nested "$RUNNER" "$TMP/reg.sh" "$TMP/n1.log" "$TMP/n1.tsv"; N1RC=$?

grep -q $'m430_probe_leaky\tFAIL' "$TMP/n1.tsv" 2>/dev/null \
    && ok "N1 负控：泄漏门被判红（TSV FAIL）" || bad "N1 泄漏门没被判红（rc=$N1RC）—— 缺陷 879 仍在"
grep -q 'PORT-LEAK gate=m430_probe_leaky' "$TMP/n1.log" && grep -q ":$PORT_LEAK" "$TMP/n1.log" \
    && ok "N1b 红文本点名泄漏门 + 端口 $PORT_LEAK" || bad "N1b 红文本没点名端口"
grep -q 'REAPED' "$TMP/n1.log" && ok "N1c 残留被回收（REAPED）" || bad "N1c 没回收残留"
listen_has "$PORT_LEAK" && bad "N1d 回收后端口仍被占（$PORT_LEAK）" || ok "N1d 回收后端口真的空了"
grep -q $'m430_probe_clean\tOK' "$TMP/n1.tsv" 2>/dev/null \
    && ok "P1 正控：干净门仍判 OK（防过度修复）" || bad "P1 干净门被误判红"
grep -q '泄漏 1' "$TMP/n1.log" && ok "N1e 汇总里泄漏计数可见（口径不静默）" || bad "N1e 汇总没报泄漏计数"

# ── P5 豁免账 ──────────────────────────────────────────────
EX="$ROOT/selfhost/port_leak_exempt.tsv"
if [ -f "$EX" ]; then
    GATE_PORTSCAN_EXEMPT="$EX" bash "$SCAN" scan --mark "no-such-marker-$$" --gate m430_probe 2>&1 | grep -q '❌' \
        && bad "P5 仓内豁免表格式错（见上）" || ok "P5 仓内豁免表格式合法（理由≥8 / 到期日 / 上限）"
else
    bad "P5 缺 selfhost/port_leak_exempt.tsv（豁免账必须存在，哪怕是空表）"
fi
printf 'm430_probe_leaky\t端到端负控用的假门名（故意泄漏，验证判据用）\t2099-12-31\n' > "$TMP/ex.tsv"
GATE_PORTSCAN_EXEMPT="$TMP/ex.tsv" bash "$SCAN" scan --mark "no-such-marker-$$" --gate m430_probe_leaky 2>&1 \
    | grep -q 'EXEMPT m430_probe_leaky' \
    && ok "P5b 命中豁免账的门不判红（EXEMPT 行可见）" || bad "P5b 豁免没生效"

# ── P6 负控自证：把判据摘除 ⇒ N1 必须变绿 ────────────────────
NEG="$TMP/negroot"
mkdir -p "$NEG/selfhost"
cp "$RUNNER" "$NEG/selfhost/run_gates.sh"
for f in gate_verdict.sh gate_skip_loud.sh; do [ -f "$ROOT/selfhost/$f" ] && ln -s "$ROOT/selfhost/$f" "$NEG/selfhost/$f"; done
sed 's/^    _check_exempt || return 2$/    _check_exempt || return 2\n    return 0  # patched: 判据被摘除/' "$SCAN" > "$NEG/selfhost/gate_portscan.sh"
if grep -q 'patched: 判据被摘除' "$NEG/selfhost/gate_portscan.sh"; then
    ok "P6a 前提自证：负控副本真被改过（判据已摘除）"
    mk_reg "$TMP/reg2.sh" m430_probe_leaky2 m430_probe_clean2
    run_nested "$NEG/selfhost/run_gates.sh" "$TMP/reg2.sh" "$TMP/n2.log" "$TMP/n2.tsv"; N2RC=$?
    grep -q $'m430_probe_leaky2\tOK' "$TMP/n2.tsv" 2>/dev/null \
        && ok "P6 负控：判据摘除后同一泄漏门变绿（证明 N1 的红来自这条判据）" \
        || bad "P6 判据摘除后仍判红（N1 的红来自别处，判据没牙）"
    LP="$(pid_of "$PORT_LEAK")"
    if [ -n "${LP:-}" ]; then
        ok "P6c 前提自证：摘除判据后残留**真的**留下来了（pid=$LP，rc=$N2RC）"
        KIDS+=("$LP"); kill -KILL "$LP" 2>/dev/null
    else
        bad "P6c 摘除后没看到残留 —— 负控场景没成立"
    fi
else
    bad "P6a 负控构造失败：补丁没打上（判据会空转）"
fi

# ── G2 零污染 ──────────────────────────────────────────────
GIT1="$(git -C "$ROOT" status --porcelain 2>/dev/null | wc -l)"
[ "$GIT0" = "$GIT1" ] && ok "G2 零污染：git status 行数 $GIT0 → $GIT1" \
                      || bad "G2 零污染被破坏：$GIT0 → $GIT1"

# ── G3 本门收尾 ────────────────────────────────────────────
LEFT=0
for p in "$PP" "$PORT_LEAK" "$PORT_OK"; do [ -n "${p:-}" ] && listen_has "$p" && LEFT=$((LEFT+1)); done
[ "$LEFT" = 0 ] && ok "G3 收尾：本门没留任何监听" || bad "G3 收尾：仍有 $LEFT 个端口被占"

echo
echo "=== 汇总：PASS $PASS / FAIL $FAIL ==="
[ "$FAIL" = 0 ] || exit 1
exit 0
