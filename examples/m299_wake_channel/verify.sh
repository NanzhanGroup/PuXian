#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M299 门：无人值守链的「自唤醒」通道（缺陷 515 + 516）
#
# 由来（用户令 2026-10-11）：
#   「01:36 那条 QQ 推送我收到了，但是那个没有意义，推给我，难道我能起来干活吗？
#     那就不应该推给我，应该是推给你，你干完活给我报告，
#     我不需要这种无人值守的脚本推送消息。」
#
#   修前：链红 / 裁决不一致 / 前提不成立 ⇒ 只挂 p1 ⇒ 45 分钟无人了结 ⇒
#         `dy-notify.sh esc` **推一条 QQ 给用户**（用户既不能也不该去修）。
#   修后：**立即调 dy-wake.sh 叫醒东月**（走 QQ 网关的网关注入通道 msg_type=inject
#         ⇒ 网关构造一条 owner 原生消息 ⇒ 完整会话链路 ⇒ 东月自己修 + 汇报）。
#         p1 仍保留 ⇒ `esc` 45 分钟后**再次唤醒**（不再推用户）；
#         只有「叫醒东月这条路本身断了」才惊动用户（基础设施事故，1 小时一次上限）。
#
# 层：
#   [1] 静态（源码派生 + 反向判据 + 规模锚点）
#   [2] 动态：观察器四分支 —— RED/INCONSISTENT/PREMISE **必须唤醒**；GREEN **必须不**唤醒
#   [3] 动态：dy-wake 去重与三重限流（真实脚本；CI 上无此文件 ⇒ 响亮 SKIP 并计数）
#   [4] 负控 2 道（各自独立判红 · 只改 $W 里的副本，**不动仓库源**）
#   [5] 覆盖边界
#
# ⚠️ 本门**不提供** `--neg-skip`：两道负控都只改工作区副本（≈1s），任何档位全跑。
# ⚠️ 纪律：不写死开发机路径（缺陷 300）· 提示文本用单引号（反引号会触发命令替换）·
#   判据只看**代码行**（注释里的历史说明不算，M251 老坑）· 不写 `大产出 | grep -q`（P3 禁形）。
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo '❌ [M276] 门级互斥锁 source 失败' >&2; exit 2; }

ROOT="${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
case "${1:-}" in ""|--*) ;; *) ROOT="$1" ;; esac
cd "$ROOT" || exit 1
W="${M299_W:-/tmp/m299_gate}"; rm -rf "$W"; mkdir -p "$W/bin" "$W/logs"

PASS=0; FAIL=0
ok(){ PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"; }
bad(){ FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$1"; }
chk(){ if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
# 去注释后的代码文本（判据只看代码行）
code(){ sed -e 's/#.*$//' "$1" 2>/dev/null; }

WC="packaging/chain/watch_chain.sh"
DN="/usr/local/sbin/dy-notify.sh"
DW="/usr/local/sbin/dy-wake.sh"
SKIP=0

printf '\n══ [1] 静态：三分支唤醒 + esc 改道 + openid 剥后缀 ══\n'
[ -f "$WC" ] || { bad "观察器不存在：$WC"; echo 'M299-VERIFY-FAIL'; exit 1; }

# 1.1 wake() 定义在位
n="$(code "$WC" | grep -c '^wake()' || true)"; chk '观察器定义了 wake()' "$n" '1'
# 1.2 三分支各调一次 wake（RED / INCONSISTENT / PREMISE 的键）
for k in red verdict premise; do
  n="$(code "$WC" | grep -c "wake \"m\$N-$k\"" || true)"
  chk "分支 $k 调用 wake" "$n" '1'
done
# 1.3 反向：唤醒调用必须在**分支体**内（不能只在函数定义里出现）
n="$(code "$WC" | grep -c 'wake ' || true)"
if [ "$n" -ge 4 ]; then ok "wake 调用点 ≥4（定义 1 + 分支 3+）"; else bad "wake 调用点仅 $n 处"; fi
# 1.4 绿色路径：**唯一允许**的唤醒是「make_tag 推送失败」的补救（那也不是结论，是行动）
#     ⚠️ 首版写成『GREEN 分支 wake 数 == 0』是**错的判据** —— 那把「发版没推上去」也
#     当成「不用叫醒」，而那恰恰需要东月去救。真正的红线由 [2] 动态层判（GREEN 正常路径不唤醒）。
n="$(code "$WC" | awk '/^GREEN\)/,/^[[:space:]]*;;/' | grep -c '^ *wake ' || true)"
if [ "$n" -le 1 ]; then ok "GREEN 分支唤醒点 ≤1（仅 make_tag 失败补救）"; else bad "GREEN 分支有 $n 处唤醒"; fi
n="$(awk '/^GREEN\)/,/^[[:space:]]*;;/' "$WC" | grep -c 'make_tag.sh' || true)"
if [ "$n" -ge 1 ]; then ok 'GREEN 分支含 make_tag（发版动作在位）'; else bad 'GREEN 分支缺 make_tag'; fi

# 1.5 失败态**接上唤醒通道**（CI 上无该文件 ⇒ SKIP）
#     ⚠️ M299s1：判据改**机制无关**。理由（如实登记）：v2.5（2026-10-11 11:12，用户令
#        「不要使用 dy-notify.sh 脚本（dy-wake.sh 那套伪造 QQ 消息），要完善文殊系统」）
#        把叫醒机制由「dy-wake.sh 伪造 QQ 消息经网关注入」换成**文殊原生任务自驱**
#        （写 handoff 交接单 + `ws-core -task-park`）。本门判的是**契约**
#        「失败态叫醒东月、且不推给用户」，**不是**某一种实现 ⇒ 两种形态都算通过。
if [ -f "$DN" ]; then
  n="$(code "$DN" | grep -c 'dy-wake.sh\|dy_wake\|task-park\|handoff' || true)"
  if [ "$n" -ge 1 ]; then ok "dy-notify 的失败态接上唤醒通道（$n 处 · 机制无关判据）"; else bad 'dy-notify 失败态未接唤醒通道'; fi
  # esc 段里不得再有「直接推用户」的合并块
  n="$(code "$DN" | awk '/^cmd_esc\(\)/,/^}/' | grep -c 'push_qq "\$t"' || true)"
  chk 'esc 不再直接合并推用户' "$n" '0'
  # 只有「通道坏了」才推用户（那条分支必须在）
  n="$(code "$DN" | grep -cE 'WAKE-FAIL|自驱失败|唤醒失败' || true)"
  if [ "$n" -ge 1 ]; then ok 'esc 保留「唤醒失败才推用户」分支'; else bad '缺「唤醒失败才推用户」分支'; fi
else
  SKIP=$((SKIP+1)); printf '  ⏭ dy-notify.sh 不在本机（%s）⇒ 静态 1.5 跳过\n' "$DN"
fi

# 1.6 dy-wake 剥 openid 后缀（516：ws-todo 的历史坑）
if [ -f "$DW" ]; then
  n="$(code "$DW" | grep -c '%@im.qq' || true)"
  if [ "$n" -ge 1 ]; then ok 'dy-wake 剥掉 @im.qq 后缀（白名单是精确比较）'; else bad 'dy-wake 未剥后缀'; fi
else
  SKIP=$((SKIP+1)); printf '  ⏭ dy-wake.sh 不在本机（%s）⇒ 静态 1.6 跳过\n' "$DW"
fi

printf '\n══ [2] 动态：观察器四分支（桩，不惊动任何人）══\n'
STUBLOG="$W/stub.log"; : > "$STUBLOG"
cat > "$W/bin/dy-notify.sh" <<SH
#!/usr/bin/env bash
printf 'NOTIFY %s\n' "\$*" >> "$STUBLOG"
exit 0
SH
cat > "$W/bin/dy-wake.sh" <<SH
#!/usr/bin/env bash
printf 'WAKE %s\n' "\$*" >> "$STUBLOG"
exit 0
SH
chmod +x "$W/bin/dy-notify.sh" "$W/bin/dy-wake.sh"

mklog(){ # $1=文件 $2=形态
  case "$2" in
    red)          printf '❌ m299_demo（1s）\n✅ m299_ok（1s）\n\n══ 汇总：失败 1 项（总耗时 2s）══\n' > "$1" ;;
    inconsistent) printf '✅ m299_ok（1s）\n\n══ 汇总：失败 2 项（总耗时 2s）══\n'                > "$1" ;;
    green)        printf '✅ m299_ok（1s）\n\n══ 汇总：失败 0 项（总耗时 2s）══\n'                > "$1" ;;
  esac
}
run_wc(){ # $1=形态  $2=额外参数（**可空**）
  # ⚠️ 两个 local 分两行写：`local a="$1" b="$W/$a"` 在 bash5.1 + set -u 下取的是**外层** a
  #    （本仓已踩过三次：M200 / M213 / M226）
  local m="$1"
  local extra="${2:-}"
  : > "$STUBLOG"
  # shellcheck disable=SC2086  # extra 需要按词拆分成多个参数
  PATH="$W/bin:$PATH" bash "$WC" 299 "$W/logs/$m.log" --no-push $extra >"$W/logs/out.$m" 2>&1
  echo "$?"
}
mklog "$W/logs/red.log" red
mklog "$W/logs/inconsistent.log" inconsistent
mklog "$W/logs/green.log" green

rc="$(run_wc red)"
chk 'RED 分支退出码 1' "$rc" '1'
n="$(grep -c '^WAKE m299-red ' "$STUBLOG" || true)"; chk 'RED ⇒ **唤醒**（键 m299-red）' "$n" '1'
n="$(grep -c '^NOTIFY p1 m299-red ' "$STUBLOG" || true)"; chk 'RED ⇒ 仍挂 p1（供 esc 重试）' "$n" '1'

rc="$(run_wc inconsistent)"
chk 'INCONSISTENT 分支退出码 2' "$rc" '2'
n="$(grep -c '^WAKE m299-verdict ' "$STUBLOG" || true)"; chk 'INCONSISTENT ⇒ 唤醒（键 m299-verdict）' "$n" '1'

rc="$(run_wc red '--runner /nonexistent/run_gates.sh')"
chk 'PREMISE 分支退出码 3' "$rc" '3'
n="$(grep -c '^WAKE m299-premise ' "$STUBLOG" || true)"; chk 'PREMISE ⇒ 唤醒（键 m299-premise）' "$n" '1'

rc="$(run_wc green)"
chk 'GREEN 分支退出码 0' "$rc" '0'
n="$(grep -c '^WAKE ' "$STUBLOG" || true)"; chk 'GREEN ⇒ **不**唤醒' "$n" '0'
n="$(grep -c '^NOTIFY p0 ' "$STUBLOG" || true)"; chk 'GREEN ⇒ p0（结论推用户）' "$n" '1'

printf '\n══ [3] 动态：dy-wake 去重与三重限流 ══\n'
if [ -x "$DW" ]; then
  # 假 push socket：回 {"ok":true}
  FS="$W/fake.sock"
  python3 - "$FS" >"$W/logs/fsrv.log" 2>&1 <<'PY' &
import socket, sys, os
p = sys.argv[1]
try: os.unlink(p)
except OSError: pass
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM); s.bind(p); s.listen(8); s.settimeout(45)
while True:
    try: c, _ = s.accept()
    except socket.timeout: break
    try:
        c.settimeout(2); c.recv(4096); c.sendall(b'{"ok":true}\n')
    except Exception: pass
    finally: c.close()
PY
  FPID=$!
  sync; sleep 0.5
  export DY_WAKE_SOCK="$FS" DY_WAKE_DIR="$W/state" DY_WAKE_LOG="$W/state/wake.log"
  export DY_WAKE_ENVF="${DY_WAKE_ENVF:-/data/app/ws/.env}"
  export DY_WAKE_COOLDOWN=1800 DY_WAKE_MAX=3 DY_WAKE_HOURLY=99
  "$DW" k1 'T' 'b' >/dev/null 2>&1; chk '首次注入 rc=0' "$?" '0'
  "$DW" k1 'T' 'b' >/dev/null 2>&1; chk '同 key 冷却 ⇒ rc=3' "$?" '3'
  "$DW" k2 'T' 'b' >/dev/null 2>&1; chk '异 key 不受冷却影响 rc=0' "$?" '0'
  DY_WAKE_COOLDOWN=0 "$DW" k3 'T' 'b' >/dev/null 2>&1
  DY_WAKE_COOLDOWN=0 "$DW" k3 'T' 'b' >/dev/null 2>&1
  DY_WAKE_COOLDOWN=0 "$DW" k3 'T' 'b' >/dev/null 2>&1
  DY_WAKE_COOLDOWN=0 "$DW" k3 'T' 'b' >/dev/null 2>&1; chk '同 key 达上限 ⇒ rc=4' "$?" '4'
  DY_WAKE_COOLDOWN=0 DY_WAKE_HOURLY=1 "$DW" k9 'T' 'b' >/dev/null 2>&1; chk '全局每小时上限 ⇒ rc=5' "$?" '5'
  kill "$FPID" 2>/dev/null
  # 台账可读
  st="$(DY_WAKE_SOCK="$FS" DY_WAKE_DIR="$W/state" "$DW" status 2>&1)"
  case "$st" in *'KEY'*) ok 'status 可读台账' ;; *) bad 'status 异常' ;; esac
else
  SKIP=$((SKIP+1)); printf '  ⏭ dy-wake.sh 不在本机（%s）⇒ 动态 3 跳过\n' "$DW"
fi
unset DY_WAKE_SOCK DY_WAKE_DIR DY_WAKE_LOG DY_WAKE_COOLDOWN DY_WAKE_MAX DY_WAKE_HOURLY 2>/dev/null || true

printf '\n══ [4] 负控（只改工作区副本，不动仓库源）══\n'
# NC-A：把 RED 分支的唤醒调用从副本里删掉 ⇒ 红线必须**不再唤醒**（判据有牙）
mkdir -p "$W/nc/packaging/chain"
cp -a packaging/chain/. "$W/nc/packaging/chain/"
python3 - "$W/nc/$WC" <<'PY'
import io, sys
p = sys.argv[1]; s = io.open(p, encoding='utf-8').read()
old = '    wake "m$N-red" "M$N 全量门红（未推送）" "红门：$RL｜门日志：$LOG"\n'
assert s.count(old) == 1, 'NC-A 锚点命中 %d 次' % s.count(old)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, ''))
PY
if [ $? = 0 ]; then
  : > "$STUBLOG"
  mkdir -p "$W/nc/logs"; cp -a "$W/logs/red.log" "$W/nc/logs/red.log"
  PATH="$W/bin:$PATH" bash "$W/nc/$WC" 299 "$W/nc/logs/red.log" --no-push >"$W/nc/out" 2>&1
  n="$(grep -c '^WAKE m299-red' "$STUBLOG" || true)"
  chk 'NC-A：删掉唤醒 ⇒ RED 不再唤醒（负控成立）' "$n" '0'
else
  bad 'NC-A 打桩失败（锚点缺失）'
fi
# NC-B：dy-wake 去掉剥后缀那行 ⇒ 用带后缀 id 时 owner 取值必带后缀（用假 socket 观察）
if [ -x "$DW" ]; then
  cp -a "$DW" "$W/nc/dy-wake.sh"
  python3 - "$W/nc/dy-wake.sh" <<'PY'
import io, sys
p = sys.argv[1]; s = io.open(p, encoding='utf-8').read()
old = 'OWNER="${OWNER%@im.qq}"\n'
assert s.count(old) == 1, 'NC-B 锚点命中 %d 次' % s.count(old)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, ''))
PY
  if [ $? = 0 ]; then
    # 用一个「.env 带后缀」的假环境文件 ⇒ 修复版剥掉、撤回版留着
    printf 'QQ_OWNER_OPENID=AAAA@im.qq\n' > "$W/nc/env"
    a="$(DY_WAKE_ENVF="$W/nc/env" DY_WAKE_DIR="$W/nc/s1" DY_WAKE_SOCK=/nonexistent "$W/nc/dy-wake.sh" --dry x 't' 'b' 2>&1)"
    b="$(DY_WAKE_ENVF="$W/nc/env" DY_WAKE_DIR="$W/nc/s2" DY_WAKE_SOCK=/nonexistent "$DW"        --dry x 't' 'b' 2>&1)"
    case "$a" in *'owner=AAAA@im.qq'*) ok 'NC-B：撤回剥后缀 ⇒ owner 带后缀（负控成立，会与白名单失配）' ;;
                *) bad "NC-B 未复现：$a" ;; esac
    case "$b" in *'owner=AAAA '*|*'owner=AAAA）'*|*'owner=AAAA') ok '修复版：owner 已剥后缀' ;;
                *) bad "修复版未剥后缀：$b" ;; esac
  else
    bad 'NC-B 打桩失败（锚点缺失）'
  fi
else
  SKIP=$((SKIP+1)); printf '  ⏭ dy-wake.sh 不在本机 ⇒ NC-B 跳过\n'
fi

printf '\n══ [5] 覆盖边界 ══\n'
cat <<'TXT'
  · 本门**不发真实注入**（会惊动用户）—— [2] 全走桩，[3] 走假 socket。
    「真通道能把我叫醒」由**人工演练**证明（M299 已做：注入 → 网关日志
    `[inject] 收到 … user=<裸 openid>` → `[inject-] 收到来自 … C2C_MESSAGE_CREATE`
    → `已创建任务 T…`），**不是**由本门保证。
  · [2] 只判「观察器是否调用 wake」；`dy-wake.sh` 内部的限流由 [3] 覆盖（两者互补）。
  · CI 上 `dy-notify.sh` / `dy-wake.sh` 不在（它们住在服务器的 /usr/local/sbin）⇒
    相关层**响亮 SKIP 并计数**，不静默绿。
  · 不覆盖：`dy-ops-escalate.timer` 的实际触发（systemd 单元本体）· QQ 网关侧的
    白名单/频控行为 · 「唤醒后东月是否真修好」。
TXT
ok '覆盖边界已登记（见上）'

printf '\n══ 汇总：通过 %d / 失败 %d（SKIP %d）══\n' "$PASS" "$FAIL" "$SKIP"
if [ "$FAIL" = 0 ]; then echo 'M299-VERIFY-OK'; rm -rf "$W"; exit 0; fi
echo 'M299-VERIFY-FAIL'
exit 1
