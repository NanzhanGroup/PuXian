#!/usr/bin/env bash
# ============================================================
# selfhost/check_m444_stream.sh —— 门档输出「边跑边转」（M444 · 缺陷 933）
# ------------------------------------------------------------
# 病史（2026-11-24 M443 收口实测）：
#   release_preflight.sh --gates 跑了 42 分钟（1867s），而调用方看到的日志
#   **全程 0 字节** —— 既看不出跑到哪一门，也判断不了「慢」还是「卡死」，
#   只能去翻 /tmp/gate_*.log 拼。根因：runner 输出被重定向进临时 $log，
#   本档只在结束时 tail -30 一次。
#
# 修法：后台 tail -f 把新增行实时转给 stderr（不接管道 ⇒ rc 仍是门自己的 rc；
#   tail 只读不写 ⇒ 不改变判据输入）。两个坑当场踩过并写进代码：
#   ① tail 的 stdout 是文件时默认全缓冲 ⇒ 必须 stdbuf -oL（否则「边跑边转」变成结束时一次吐出）
#   ② `sp="$(strm_start …)"` 会等后台子壳退出（继承替换管道）⇒ 用全局变量带回 PID
#
# 判据：
#   S1 形状：本档有 stdbuf -oL tail -f；有 PREFLIGHT_STREAM_OFF 开关；tail 的 PID 被 kill+wait 收尾
#   P1 正控：--self-test 的 G-P3 四条全 ✅（流式正控 / 前提自证 / 负控前提自证 / 负控）
#   P2 整体：--self-test 无 ❌ 行，且通过数 = 总数
#   N1 负控（--negctl）：把 stdbuf -oL 打掉 ⇒ G-P3 正控必须变 ❌（证明这条判据有牙）
#   G1 环境守卫：release_preflight.sh 不可读 / bash 不可用 ⇒ rc=3
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -uo pipefail
cd "$(cd "$(dirname "$0")/.." && pwd)"
PF="./packaging/release_preflight.sh"
PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

echo "=== m444_stream · 门档输出边跑边转（缺陷 933）==="
[ -f "$PF" ] || { echo "❌ 环境不成立：$PF 不存在" >&2; exit 3; }
command -v bash >/dev/null 2>&1 || { echo "❌ 环境不成立：缺 bash" >&2; exit 3; }

if [ "${1:-}" = "--negctl" ]; then
    BAK="$(mktemp)"; cp -f "$PF" "$BAK"
    trap 'cp -f "$BAK" "$PF"; rm -f "$BAK"' EXIT
    echo "=== --negctl：把流式默认关掉（PREFLIGHT_STREAM_OFF 默认 1）==="
    sed -i 's@PREFLIGHT_STREAM_OFF:-0@PREFLIGHT_STREAM_OFF:-1@' "$PF"
    grep -q 'PREFLIGHT_STREAM_OFF:-1' "$PF" && echo "  打桩已落地（前提自证：默认值已改成 1）" || { echo "❌ 打桩没生效"; exit 1; }
    out="$(timeout 300 bash "$PF" --self-test 2>&1)"
    if printf '%s\n' "$out" | grep -q '❌ G-P3 非流式'; then
        echo "M444-STREAM NEG-CONTROL OK（默认关掉流式后正控当即变红 ⇒ 判据有牙）"
        exit 0
    fi
    echo "❌ 打桩后 G-P3 仍绿 ⇒ 正控没有牙（它测不到『有没有真的边跑边转』）"
    printf '%s\n' "$out" | grep 'G-P3' | sed 's/^/    /'
    exit 1
fi

# ── S1 形状 ──
grep -q 'tail -n +1 -f "\$log" 1>&2 &' "$PF" && ok "S1a 形状：tail -f 把新增行转发到 stderr（每门完成即可见）" \
                                           || bad "S1a 没有 tail -f 转发（输出仍攒到结束）"
grep -q 'PREFLIGHT_STREAM_OFF' "$PF" && ok "S1b 形状：有 PREFLIGHT_STREAM_OFF 开关（负控靠它）" \
                                    || bad "S1b 缺 PREFLIGHT_STREAM_OFF 开关"
grep -q 'kill "\$_tp"' "$PF" && grep -q 'wait "\$_tp"' "$PF" \
    && ok "S1c 形状：tail 的 PID 被 kill + wait 收尾（不留孤儿进程）" || bad "S1c tail 没被收尾"

# ── P1/P2 跑自测 ──
out="$(timeout 300 bash "$PF" --self-test 2>&1)"; rc=$?
[ "$rc" = 0 ] && ok "P2 --self-test rc=0" || bad "P2 --self-test rc=$rc"
printf '%s\n' "$out" | grep -q '❌' && bad "P2 自测里有 ❌ 行" || ok "P2 自测无 ❌ 行"
n_ok="$(printf '%s\n' "$out" | grep -c '✅')"
n_line="$(printf '%s\n' "$out" | grep -cE '^  (✅|❌)')"
echo "  扫描面：自测断言 $n_line 条（✅ $n_ok）"
if [ "$n_ok" -ge 20 ] && [ "$n_ok" = "$n_line" ]; then
    ok "P2 自测通过数=断言数（$n_ok/$n_line，无静默跳过）"
else
    bad "P2 自测通过 $n_ok / 断言 $n_line —— 有失败或缺断言（规模下限 20）"
fi
for k in "G-P3 流式正控" "G-P3 前提自证" "G-P3 负控前提自证" "G-P3 负控"; do
    printf '%s\n' "$out" | grep -q "✅ $k" && ok "P1 $k：✅" || bad "P1 $k：未通过或缺失"
done

echo "── 合计：通过 $PASS · 失败 $FAIL ──"
[ "$FAIL" = 0 ] && { echo "M444-STREAM OK n=$PASS"; exit 0; }
echo "M444-STREAM FAIL pass=$PASS fail=$FAIL"
exit 1
