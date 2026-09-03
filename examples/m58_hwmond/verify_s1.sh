#!/usr/bin/env bash
# ============================================================
# M58 S1 verify —— pxhwmond 骨架 + /proc 采集（多文件 import 工程）
# ------------------------------------------------------------
# 验证点：
#   A. 多文件 import 工程编译为静态 ELF（main.px import collect.px）
#   B. 每 .px 文件行数 < 500（对大模型友好的工程约束）
#   C. --once 单轮自检输出 SNAP 行，字段齐全
#   D. mem_total 与 `grep MemTotal /proc/meminfo` 精确对拍（稳定值）
#      mem_avail 合理（0 < avail < total）；cpu ∈ 0..100 或 na
#   E. load1/5/15 与 `cat /proc/loadavg` 前 3 字段格式一致且差值 < 1.0
#      （load 是 1/5/15 分钟均值，两次读取间隔内变化极小，宽松容差防竞态）
#   F. up 与 `cat /proc/uptime` 第一字段差值 < 30s（运行瞬间差，宽松容差）
#   G. --n 2 --interval 1 有限时长冒烟：两轮 SNAP、两轮 mem_total 一致
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m58_hwmond/build
D=examples/m58_hwmond

fail() { echo "M58-S1 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. build（多文件 import 工程）=="
$PXC build "$D/main.px" >/dev/null 2>&1 || fail "main.px 编译失败"
[ -x "$B/main" ] || fail "产物 $B/main 不存在"
ARCH=$(file "$B/main")
echo "$ARCH"
echo "$ARCH" | grep -q "x86-64" || fail "产物不是 x86-64"
echo "$ARCH" | grep -q "statically linked" || fail "产物非静态链接"
ok "编译成功：静态 ELF"

echo "== B. 每文件行数 < 500 =="
for f in "$D"/main.px "$D"/collect.px; do
    L=$(wc -l < "$f")
    echo "  $f: $L 行"
    [ "$L" -lt 500 ] || fail "$f 超 500 行（$L）"
done
ok "行数约束满足"

echo "== C. --once 单轮自检 =="
OUT=$("$B/main" --once 2>&1) || fail "--once 退出码非 0"
echo "$OUT"
SNAP=$(echo "$OUT" | grep '^SNAP ')
[ -n "$SNAP" ] || fail "无 SNAP 输出行"
echo "$SNAP" | grep -q "ts=[0-9]\+" || fail "缺 ts 字段"
ok "--once 输出 SNAP 行（含 ts）"

# 提取字段（顺序固定：ts cpu mem_total mem_avail load1 load5 load15 up）
CPU=$(echo "$SNAP" | sed 's/.* cpu=\([^ ]*\).*/\1/')
MEM_TOTAL=$(echo "$SNAP" | sed 's/.* mem_total=\([^ ]*\).*/\1/')
MEM_AVAIL=$(echo "$SNAP" | sed 's/.* mem_avail=\([^ ]*\).*/\1/')
LOAD1=$(echo "$SNAP" | sed 's/.* load1=\([^ ]*\).*/\1/')
LOAD5=$(echo "$SNAP" | sed 's/.* load5=\([^ ]*\).*/\1/')
LOAD15=$(echo "$SNAP" | sed 's/.* load15=\([^ ]*\).*/\1/')
UP=$(echo "$SNAP" | sed 's/.* up=\([^ ]*\).*/\1/')

echo "== D. mem 对拍（稳定值精确）=="
REF_MEM=$(awk '/^MemTotal:/{print $2}' /proc/meminfo)
[ -n "$REF_MEM" ] || fail "无法读取 /proc/meminfo MemTotal"
[ "$MEM_TOTAL" = "$REF_MEM" ] || fail "mem_total=$MEM_TOTAL 应=$REF_MEM"
ok "mem_total=$MEM_TOTAL 与 MemTotal 精确一致"
echo "$MEM_AVAIL" | grep -qE '^[0-9]+$' || fail "mem_avail 非数字: $MEM_AVAIL"
[ "$MEM_AVAIL" -gt 0 ] && [ "$MEM_AVAIL" -lt "$MEM_TOTAL" ] || fail "mem_avail 越界: $MEM_AVAIL"
ok "mem_avail=$MEM_AVAIL 合理（0<avail<total）"

echo "== E. cpu 断言 =="
if [ "$CPU" = "na" ]; then
    echo "  cpu=na（差值窗口无 tick，允许）"
else
    echo "$CPU" | grep -qE '^[0-9]+$' || fail "cpu 非数字: $CPU"
    [ "$CPU" -ge 0 ] && [ "$CPU" -le 100 ] || fail "cpu 越界 0..100: $CPU"
    ok "cpu=$CPU% ∈ 0..100"
fi

echo "== F. load / up 与 cat /proc 对拍（宽松容差防竞态）=="
read -r RL1 RL5 RL15 _ < /proc/loadavg
for v in "$LOAD1" "$LOAD5" "$LOAD15" "$RL1" "$RL5" "$RL15"; do
    echo "$v" | grep -qE '^[0-9]+([.][0-9]+)?$' || fail "load 值非浮点: $v"
done
D1=$(awk -v a="$LOAD1" -v b="$RL1" 'BEGIN{d=a-b;if(d<0)d=-d;printf "%.3f",d}')
D5=$(awk -v a="$LOAD5" -v b="$RL5" 'BEGIN{d=a-b;if(d<0)d=-d;printf "%.3f",d}')
D15=$(awk -v a="$LOAD15" -v b="$RL15" 'BEGIN{d=a-b;if(d<0)d=-d;printf "%.3f",d}')
awk -v d="$D1" 'BEGIN{exit !(d<1.0)}' || fail "load1 差 $D1 ≥ 1.0（解析异常？）"
awk -v d="$D5" 'BEGIN{exit !(d<1.0)}' || fail "load5 差 $D5 ≥ 1.0"
awk -v d="$D15" 'BEGIN{exit !(d<1.0)}' || fail "load15 差 $D15 ≥ 1.0"
ok "load1/5/15=$LOAD1/$LOAD5/$LOAD15 与 /proc/loadavg（$RL1/$RL5/$RL15）一致（差 <1.0）"
echo "$UP" | grep -qE '^[0-9]+([.][0-9]+)?$' || fail "up 非浮点: $UP"
RUP=$(awk '{print $1}' /proc/uptime)
DU=$(awk -v a="$UP" -v b="$RUP" 'BEGIN{d=a-b;if(d<0)d=-d;printf "%.3f",d}')
awk -v d="$DU" 'BEGIN{exit !(d<30.0)}' || fail "up 差 $DU ≥ 30s"
ok "up=$UP 与 /proc/uptime（$RUP）一致（差 <30s）"

echo "== G. --n 2 --interval 1 冒烟 =="
OUT2=$("$B/main" --n 2 --interval 1 2>&1) || fail "--n 2 退出码非 0"
N2=$(echo "$OUT2" | grep -c '^SNAP ')
[ "$N2" -eq 2 ] || fail "应 2 行 SNAP，实际 $N2"
echo "$OUT2"
T1=$(echo "$OUT2" | sed -n '1s/.* mem_total=\([^ ]*\).*/\1/p')
T2=$(echo "$OUT2" | sed -n '2s/.* mem_total=\([^ ]*\).*/\1/p')
[ "$T1" = "$T2" ] && [ "$T1" = "$REF_MEM" ] || fail "两轮 mem_total 不一致或与 MemTotal 不符"
ok "两轮 mem_total 一致且正确（多轮循环稳定）"

echo "✅ M58-S1: pxhwmond 骨架 + /proc 采集 PASS"
