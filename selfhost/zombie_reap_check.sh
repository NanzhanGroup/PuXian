#!/usr/bin/env bash
# ============================================================
# 子进程兜底回收门（zombie reap）—— Issue 54
# ------------------------------------------------------------
# 契约（五断言）：
#   A. 默认轨（tools/px = 用户面 VM 引擎）：os_spawn 后不 os_wait ⇒ 僵尸数必须 == 0
#   B. C 轨（tools/px --c 逃生舱）：同断言 —— 回收在 runtime 层，必须与轨无关
#   C. 反证：PX_NO_CHILD_REAP=1 关掉兜底回收 ⇒ 僵尸数必须 == 5（**门必须能红**）
#   D. 语义回归：兜底线程已收走状态后，os_wait(pid) 仍须返回 3 / 143 / 0
#   E. **解释器轨**（`px run` → bootstrap/pxi，内嵌 runtime）：同 A/B 断言
#      （M114-S2 / Issue 55 新增：pxi 与 pxc_vm 是两套二进制，修复须分别重烘）
#
# 为什么要有这条门（Issue 54 · 现网实证）：
#   晨曦 chenxi 的 Mahesvara ma-sec 每次封禁派生一个 ma-alertd（投递告警后立即退出），
#   调用方从不 os_wait ⇒ 每个子进程变成僵尸并**永久驻留**：ma-sec 自 2026-09-11 23:39:53
#   启动后 5 次封禁 = 5 个 [ma-alertd] <defunct>，只有重启 ma-sec 才清零（1:1，日志逐条对应）。
#   语言侧根因：os_wait 是**阻塞**的 —— "派生后不管"这一常见用法根本没有可用的回收原语。
#
# 退出码：0 = 全部门通过；1 = 有分歧
# 用法：./zombie_reap_check.sh
# ============================================================
set -u
cd "$(dirname "$0")/.."
ROOT=$(pwd)
PXDEF="$ROOT/tools/px"
LEAK="$ROOT/selfhost/probes/zombie_reap_leak.px"
STASH="$ROOT/selfhost/probes/zombie_reap_stash.px"
WORK=${TMPDIR:-/tmp}/px_zombie_reap
EXPECT_LEAK=5     # 反证期望值：探针派生 5 个子进程

rm -rf "$WORK"; mkdir -p "$WORK/vm" "$WORK/cc"

[ -x "$PXDEF" ] || { echo "❌ 缺少默认轨入口 $PXDEF" >&2; exit 1; }
[ -e "$LEAK" ] || { echo "❌ 缺少探针 $LEAK" >&2; exit 1; }
[ -e "$STASH" ] || { echo "❌ 缺少探针 $STASH" >&2; exit 1; }

# 跑探针二进制，返回其僵尸子进程数（探针内 sleep 保证窗口内可观测）
zombies_of() {  # $1=binary  $2=env 赋值（可空）
    local bin=$1 envs=${2:-} pid z
    env $envs "$bin" >"$WORK/out.txt" 2>&1 &
    pid=$!
    sleep 2
    z=$(ps --ppid "$pid" -o stat= 2>/dev/null | grep -c '^Z')
    wait "$pid" 2>/dev/null
    echo "${z:-0}"
}

# 解释器轨：`px run <src>` 的僵尸数（僵尸挂在内层 pxi 下，不是 bash 包装层）
run_zombies() {  # $1=.px 探针 → 僵尸数
    local src=$1 pid p z=0
    "$PXDEF" run "$src" >"$WORK/run.out" 2>&1 &
    pid=$!
    sleep 2
    for p in $(pgrep -P "$pid" 2>/dev/null); do
        z=$((z + $(ps --ppid "$p" -o stat= 2>/dev/null | grep -c '^Z')))
    done
    [ "$z" -eq 0 ] && z=$(ps --ppid "$pid" -o stat= 2>/dev/null | grep -c '^Z')
    wait "$pid" 2>/dev/null
    printf '%s' "${z:-0}"
}

bad=0
chk() {  # $1=ok(0/1) $2=标签 $3=实测 $4=期望
    if [ "$1" -eq 0 ]; then
        printf '    ✅ %s（实测 %s）\n' "$2" "$3"
    else
        printf '    ❌ %s（实测 %s，期望 %s）\n' "$2" "$3" "$4"
        bad=$((bad + 1))
    fi
}

echo "── 子进程兜底回收门（Issue 54）──"

# ---- 构建（分离目录，避免两轨同名产物互相覆盖）----
cp "$LEAK" "$STASH" "$WORK/vm/"
cp "$LEAK" "$WORK/cc/"
echo "[构建] 默认轨（VM）"
"$PXDEF" build "$WORK/vm/zombie_reap_leak.px"  >"$WORK/b_vm.log" 2>&1 || { echo "❌ 默认轨构建失败"; tail -5 "$WORK/b_vm.log"; exit 1; }
"$PXDEF" build "$WORK/vm/zombie_reap_stash.px" >>"$WORK/b_vm.log" 2>&1 || { echo "❌ 默认轨构建失败（stash）"; tail -5 "$WORK/b_vm.log"; exit 1; }
echo "[构建] C 轨（逃生舱）"
"$PXDEF" build --c "$WORK/cc/zombie_reap_leak.px" >"$WORK/b_c.log" 2>&1 || { echo "❌ C 轨构建失败"; tail -5 "$WORK/b_c.log"; exit 1; }

BIN_VM="$WORK/vm/build/zombie_reap_leak"
BIN_STASH="$WORK/vm/build/zombie_reap_stash"
BIN_C="$WORK/cc/build/zombie_reap_leak"
for b in "$BIN_VM" "$BIN_STASH" "$BIN_C"; do
    [ -x "$b" ] || { echo "❌ 未产出可执行文件：$b" >&2; exit 1; }
done

# ---- A. 默认轨：不 os_wait ⇒ 僵尸必须为 0 ----
z_vm=$(zombies_of "$BIN_VM")
[ "$z_vm" -eq 0 ]; chk $? "A 默认轨 fire-and-forget 无僵尸残留" "$z_vm" 0

# ---- B. C 轨：同断言（轨无关）----
z_c=$(zombies_of "$BIN_C")
[ "$z_c" -eq 0 ]; chk $? "B C 轨 fire-and-forget 无僵尸残留" "$z_c" 0

# ---- C. 反证：关掉兜底回收，门必须能红 ----
z_off=$(zombies_of "$BIN_VM" "PX_NO_CHILD_REAP=1")
[ "$z_off" -eq "$EXPECT_LEAK" ]; chk $? "C 反证（PX_NO_CHILD_REAP=1）确实泄漏" "$z_off" "$EXPECT_LEAK"

# ---- D. 语义回归：状态箱必须让 os_wait 拿到正确退出码 ----
"$BIN_STASH" >"$WORK/stash.out" 2>&1
got=$(grep -E '^[0-9]+$' "$WORK/stash.out" | tr '\n' ' ' | sed 's/ *$//')
want="3 143 0"
[ "$got" = "$want" ]; chk $? "D os_wait 状态箱语义（exit/signal/直接认领）" "$got" "$want"

# ---- E. 解释器轨（`px run` → bootstrap/pxi）：M114-S2（Issue 55）新增 ----
#   单列的原因：`px run` 与 `px build` 是**两套二进制** —— build 走 tools/px →
#   bootstrap/pxc_vm（生成产物，runtime 在产物里）；run 走 bootstrap/pxi（**内嵌** runtime）。
#   Issue 54 的修复因此必须**分别**重烘才生效。实测（重烘前）：`px run` 留 5 个 <defunct>
#   而本门 A/B/C/D 全绿 —— 又一次「门看不见用户真实走的那条路」（同 Issue 51/61 族）。
z_run=$(run_zombies "$LEAK")
[ "$z_run" -eq 0 ]; chk $? "E 解释器轨（px run）fire-and-forget 无僵尸残留" "$z_run" 0

echo "───────────────────────────────"
if [ "$bad" -eq 0 ]; then
    echo "🎉 门通过：5/5（僵尸 vm 0 / c 0 / run $z_run；反证泄漏 $z_off == $EXPECT_LEAK；状态箱 3 143 0）"
    exit 0
fi
echo "❌ 门失败：$bad 项不一致"
echo "    out.txt / stash.out / *.log 在 $WORK"
exit 1
