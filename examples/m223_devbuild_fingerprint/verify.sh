#!/usr/bin/env bash
# ============================================================
# M223 门（第 101 轮）：devbuild **源码链指纹的稳定源**
#
# 主题：M221 给 `selfhost/devbuild.sh` 加了产物指纹短路（实测省 ~17 min/轮），
#   但指纹本身用 `stat -c '%n %s %Y'`（**含 mtime**）⇒ 内容一字未改、只是 mtime 变
#   （`touch` / `cp` / 解包 / 编辑器"重写但没改"）就换一个 key ⇒ **后面 22 门白付冷重建**。
#   M222 补的台账把它量化了出来（CI 实测「key 3 个」），但「3 个」不说明**为什么**
#   ⇒ 本轮同时补上**归因**能力。
#
# 缺陷：
#   323  `src_line` 用 mtime 三元组 ⇒ 一次 `touch` 即触发整轮冷重建。
#        实测（/tmp/m223/exp1.sh）：mtime 版 **152ms** ⇄ 内容哈希版 **34ms**
#        —— 逐文件 `stat` 要 fork 115 次，而 `sha256sum <glob>` 一个进程全办
#        ⇒ **更严、且更快**（旧口径连"性能"这个理由都不成立）。
#   324  `CACHE` 选料的 `[ "$n" -ge 15 ]` 是**无解释的魔法数**：
#        本机 3149 个 .rtcache 目录的 .o 数实测分布
#          13×2255 · 14×415 · **15×33** · 17×228 · 18×39 · 22×34 · 26×2 · 29×143
#        ⇒ 阈值正卡在 14/15 之间（33 个目录**恰在边界**），而 13/14 的 2670 个是
#        **裁剪版**（cuts 不同）。判「个数」而不判「是哪一份」⇒ 某个合法配置只要
#        产出 14 个 .o，主路径就被**静默毙掉**、改走 mtime 回退（挑到**别的**目录）
#        ⇒ 指纹漂移，且错误信息指不到根因（M169 原话「不能靠谁最新」，同族）。
#        修法：判据改 `rt_ensure` 自己的 `.complete`（**成功才 touch**）；回退必须
#        **响亮**，并与 `tools/px rtkey`（M153「只算 key 不编译」）比对 ⇒ 选料**可判定**。
#   325  台账只报「出现过的 key N 个」⇒ 漂移**无法归因**。修法：key **取值** + 按件
#        分组 + 新增「选料来源」列 + **key 变了就 diff 逐文件清单、指名变了哪个文件**。
#
# ⚠️ 覆盖边界（如实）：见第 ⑦ 层。
#   用法：bash examples/m223_devbuild_fingerprint/verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

D="$ROOT/selfhost/devbuild.sh"
TMPD="$ROOT/selfhost/devbuild.sh.m223tmp"
TS="selfhost/parser.px"          # 探针目标（内容 ~52KB，哈希便宜）
W=/tmp/m223_gate; rm -rf "$W"; mkdir -p "$W"
SNAP="$W/snap"
pass=0; fail=0
note() { echo "   $*"; }
ok()   { echo "✅ $*"; pass=$((pass+1)); }
bad()  { echo "❌ $*"; fail=$((fail+1)); }
hdr()  { echo; echo "── $*"; }

snap_all() { mkdir -p "$SNAP"; cp -a "$D" "$SNAP/devbuild.sh.snap"; cp -a "$TS" "$SNAP/parser.px.snap"; }
restore_all() {
    [ -f "$SNAP/devbuild.sh.snap" ] && cp -a "$SNAP/devbuild.sh.snap" "$D"
    [ -f "$SNAP/parser.px.snap" ]   && cp -a "$SNAP/parser.px.snap"   "$TS"
    rm -f "$TMPD"
}
# 中途被杀也能还原（M215/M220 教训）
trap 'restore_all' EXIT

devkey() { grep -m1 '源码链指纹：' "$1" 2>/dev/null | sed 's/.*：//' | awk '{print $1}'; }
DV_ENV=()   # M287s2：可临时追加环境（见 [3]）；空数组 ⇒ 行为与以前一致
run_dev() { timeout 900 env "${DV_ENV[@]}" bash "$1" pxc > "$2" 2>&1; echo $?; }

# ── 层②的判据抽成函数：负控 C 要用「自伤档」重调它，证明"红来自比对" ──
judge_touch() {   # $1=基线日志 $2=touch 后日志 → rc 0=通过（即"没红"）
    local k0 k1 r b
    k0=$(devkey "$1"); k1=$(devkey "$2")
    r=$(grep -c '^⏭' "$2"); b=$(grep -c '^✅' "$2")
    [ "$k0" = "$k1" ] && [ "$r" -ge 1 ] && [ "$b" -eq 0 ]
}
chk_touch() {   # 受 M223_IGNORE_REUSE 控制（自伤档 ⇒ 恒"通过"）
    if [ "${M223_IGNORE_REUSE:-0}" = "1" ]; then return 0; fi
    judge_touch "$1" "$2"
}
patch_mtime() {   # 把指纹退回 mtime 版（负控 A / 自伤底座共用）
    python3 - "$D" <<'PY'
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = "        sha256sum $SRC_PATTERNS 2>/dev/null | awk '{print $2, substr($1,1,16)}'"
assert s.count(old) == 1, "锚点不唯一（内容哈希行）"
new = '        for f in $SRC_PATTERNS; do [ -f "$f" ] && stat -c \'%n %s %Y\' "$f"; done'
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
}

echo "══ M223 门：devbuild 源码链指纹的稳定源 ══"
MAX0=$SECONDS
snap_all

# ───────────────────────── ① 静态 ─────────────────────────
hdr "[1/7] 静态：逐条对应一个具体缺陷"
ck() { if grep -qF -e "$2" "$3"; then ok "$1"; else bad "$1（未命中：$2）"; fi; }
ck "指纹 = 源码**内容**哈希（缺陷 323）" 'sha256sum $SRC_PATTERNS 2>/dev/null | awk' "$D"
ck "源码链 glob 提为 SRC_PATTERNS"       'SRC_PATTERNS="selfhost/*.px runtime/*.c runtime/*.h tools/*.px stdlib/*.px"' "$D"
ck "manifest 函数 src_manifest 在位"      'src_manifest() {' "$D"
ck "三方资产用 %n %s（去 mtime）"          "stat -c '%n %s' \"\$f\"" "$D"
# ⚠️ 只查**代码行**：本文件头/函数头注里保留了「修前用 stat %Y」的历史说明，
#    直接 grep 会把注释判成违例（首跑实测踩中 ⇒ 假红）。判据要贴着被测对象写。
if grep -E "stat -c '%n %s %Y'" "$D" | grep -qvE '^[[:space:]]*#'; then
    bad "src_line 仍在用 mtime 三元组（%n %s %Y）"
else
    ok "无 mtime 三元组残留（缺陷 323 正面判据；注释里的历史说明不计）"
fi
ck "选料判据改 .complete（缺陷 324）"     '[ -f "$keyed/.complete" ]' "$D"
if grep -q '"\$n" -ge 15' "$D"; then
    bad "魔法数「.o 数 ≥ 15」仍在（缺陷 324 未修）"
else
    ok "魔法数「.o 数 ≥ 15」已移除（缺陷 324 正面判据）"
fi
ck "回退必须响亮"                         '主路径失效 ⇒ **回退**' "$D"
ck "回退后与 rtkey 比对（可判定）"        'cur_rtkey="$("$ROOT/tools/px" rtkey' "$D"
ck "归因：key 变了 diff 清单（缺陷 325）" '源码链指纹变化' "$D"
ck "台账新增「选料来源」列"               'CACHE_SRC' "$D"
ck "汇总打印 key 取值"                    '源码链 key（按件分组' "$D"

# ───────────────────────── ② mtime 不敏感（核心）─────────────────────────
hdr "[2/7] 动态：touch（只改 mtime）⇒ **必须仍复用**（缺陷 323 正面判据）"
rm -f /tmp/devbuild_pxc.fp /tmp/devbuild_pxc.src
rc0=$(run_dev "$D" "$W/base.log"); K0=$(devkey "$W/base.log")
note "基线：rc=$rc0 key=$K0（建 $(grep -c '^✅' "$W/base.log") / 复用 $(grep -c '^⏭' "$W/base.log")）"
[ "$rc0" = "0" ] && ok "基线跑通" || bad "基线 rc=$rc0"

touch "$TS"
rc1=$(run_dev "$D" "$W/touch.log"); K1=$(devkey "$W/touch.log")
REUSE=$(grep -c '^⏭' "$W/touch.log"); BUILD=$(grep -c '^✅' "$W/touch.log")
note "touch 后：rc=$rc1 key=$K1（建 $BUILD / 复用 $REUSE）"
if chk_touch "$W/base.log" "$W/touch.log"; then
    ok "touch（内容未改）⇒ key 不变 **且复用**（M223 前此处必然换 key + 重建）"
else
    bad "touch 触发了重建或 key 漂移（key $K0→$K1, 建$BUILD 复用$REUSE）⇒ 缺陷 323 仍在"
fi

# ───────────────────────── ③ 内容敏感 + 归因 ─────────────────────────
hdr "[3/7] 动态：改**一个字节** ⇒ 必须重建 + **归因指名**（缺陷 325 正面判据）"
# ⚠️ M287s2：本条测的是**源码链指纹层**（含「为什么重建」的归因），而 M287 的多槽缓存对
#   **同一份内容**会命中（⏭）⇒ 既无重建、也无归因（实测：`或未重建（建 0）` + `归因未指名`）。
#   ⇒ 显式关多槽，回到「指纹层单独作用」的场景。
#   ⚠️ 这不是「放水」：多槽层的正确性由 examples/m287_devbuild_slots/ **独立**验证
#      （45 断言 + 3 道负控）；本条要保的是**归因能力**，那道判据只有走指纹层才可达。
DV_ENV=(DEVB_SLOTS=0)
printf '\n# M223-GATE-PROBE\n' >> "$TS"
rc2=$(run_dev "$D" "$W/change.log"); K2=$(devkey "$W/change.log")
DV_ENV=()
B2=$(grep -c '^✅' "$W/change.log")
note "改内容后：rc=$rc2 key=$K2（建 $B2）"
if [ "$K2" != "$K0" ] && [ "$B2" -ge 1 ]; then
    ok "内容变 ⇒ key 变且重建（不会漏报）"
else
    bad "内容变后 key=$K2（应 ≠ $K0）或未重建（建 $B2）"
fi
if grep -q '源码链指纹变化' "$W/change.log" && grep -q '< selfhost/parser.px' "$W/change.log" \
   && grep -q '> selfhost/parser.px' "$W/change.log"; then
    ok "归因输出了逐文件 diff 且**指名 selfhost/parser.px**"
else
    bad "归因未指名 parser.px（附：$(grep -A3 '指纹变化' "$W/change.log" | tr '\n' '|')）"
fi
restore_all
rc3=$(run_dev "$D" "$W/back.log"); K3=$(devkey "$W/back.log")
if [ "$K3" = "$K0" ]; then
    ok "还原内容后 key 回到基线 $K0（mtime 已不参与 ⇒ 后续门直接复用）"
else
    bad "还原后 key=$K3 ≠ 基线 $K0（内容哈希口径下不应发生）"
fi

# ───────────────────────── ④ 回退档：可判定 ─────────────────────────
hdr "[4/7] 动态：主路径失效 ⇒ 回退必须**响亮**且**可判定**"
cp -a "$D" "$TMPD"
python3 - "$TMPD" <<'PY' || bad "回退档打桩失败"
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = 'keyed="$("$ROOT/tools/px" rtcache 2>/dev/null | tail -1)"'
assert s.count(old) == 1, "锚点不唯一（rtcache 调用）"
open(p, "w", encoding="utf-8").write(
    s.replace(old, 'keyed=""   # M223-GATE: 强制主路径失效（自证用）', 1))
PY
timeout 900 bash "$TMPD" pxc > "$W/fb.log" 2>&1; rcf=$?
rm -f "$TMPD"
if grep -q '主路径失效' "$W/fb.log"; then
    ok "回退**响亮**（打印了「主路径失效 ⇒ 回退」，rc=$rcf）"
else
    bad "回退未提示（rc=$rcf）"
fi
if grep -qE '回退挑中的 .* (正好是|≠) 当前源码 rt_key' "$W/fb.log"; then
    ok "回退后与「当前源码 rt_key」做了比对 ⇒ 选料**可判定**"
else
    bad "回退未做 rt_key 比对（日志 $(wc -l < "$W/fb.log") 行）"
fi

# ───────────────────────── ⑤ 台账可归因 ─────────────────────────
hdr "[5/7] 台账：key 取值 + 按件分组 + 选料来源（缺陷 325）"
tail -2 /tmp/devbuild_stats.tsv 2>/dev/null | sed 's/^/     /'
NF=$(awk -F'\t' '{print NF}' /tmp/devbuild_stats.tsv 2>/dev/null | sort -u | tr '\n' ' ')
if printf '%s' "$NF" | grep -qw '5'; then
    ok "台账含第 5 列（选料来源）；字段数集合=$NF"
else
    bad "台账无第 5 列（字段数集合：$NF）"
fi
SUM="$(timeout 300 bash "$D" 2>&1 | grep -A8 'devbuild 累计' || true)"
if printf '%s' "$SUM" | grep -q '源码链 key（按件分组'; then
    ok "汇总打印 **key 取值**（不只是个数）"
    printf '%s\n' "$SUM" | sed 's/^/     /'
else
    bad "汇总未打印 key 取值"
fi

# ───────────────────────── ⑥ 负控 ─────────────────────────
hdr "[6/7] 负控（各自独立判红）"
if [ "$NEG_SKIP" = "0" ]; then
    # NC-A：指纹退回 mtime ⇒ ② 的判据必须红
    restore_all; snap_all
    patch_mtime || bad "负控 A 打桩失败"
    rm -f /tmp/devbuild_pxc.fp /tmp/devbuild_pxc.src
    run_dev "$D" "$W/na0.log" >/dev/null
    touch "$TS"
    run_dev "$D" "$W/na1.log" >/dev/null
    note "负控 A 下 touch：key $(devkey "$W/na0.log") → $(devkey "$W/na1.log")（建 $(grep -c '^✅' "$W/na1.log")）"
    if judge_touch "$W/na0.log" "$W/na1.log"; then
        bad "负控 A 未判红（退回 mtime 后 touch 竟然没换 key ⇒ ② 的判据没牙）"
    else
        ok "负控 A 判红（退回 mtime ⇒ touch 即换 key ⇒ ② 的判据有牙）"
    fi
    restore_all
    cmp -s "$SNAP/devbuild.sh.snap" "$D" || bad "负控 A 还原后 devbuild.sh 不一致"

    # NC-B：抽掉归因输出 ⇒ ③ 的归因判据必须红
    restore_all; snap_all
    python3 - "$D" <<'PY' || bad "负控 B 打桩失败"
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = '            echo "── $name：源码链指纹变化 $_oldkv → $DEVB_KEY（选料=$CACHE_SRC）"'
assert s.count(old) == 1, "锚点不唯一（归因打印）"
open(p, "w", encoding="utf-8").write(s.replace(old, '            : # M223-GATE-NC-B', 1))
PY
    printf '\n# M223-GATE-NCB\n' >> "$TS"
    run_dev "$D" "$W/nb.log" >/dev/null
    if grep -q '< selfhost/parser.px' "$W/nb.log"; then
        bad "负控 B 未判红（抽掉归因后仍指名了文件）"
    else
        ok "负控 B 判红（抽掉归因 ⇒ ③ 的「指名文件」不再成立）"
    fi
    restore_all
    cmp -s "$SNAP/devbuild.sh.snap" "$D" || bad "负控 B 还原后 devbuild.sh 不一致"

    # NC-C：判据自伤 ⇒ NC-A 的红必须**消失**（证红来自比对，而不是别处）
    restore_all; snap_all
    patch_mtime || bad "负控 C 打桩失败"
    rm -f /tmp/devbuild_pxc.fp /tmp/devbuild_pxc.src
    run_dev "$D" "$W/nc0.log" >/dev/null
    touch "$TS"
    run_dev "$D" "$W/nc1.log" >/dev/null
    if judge_touch "$W/nc0.log" "$W/nc1.log"; then
        bad "负控 C 底座异常（退回 mtime 竟然还通过 —— NC-A 的红另有来源）"
    else
        if M223_IGNORE_REUSE=1 chk_touch "$W/nc0.log" "$W/nc1.log"; then
            ok "负控 C 判红（自伤档下**同一故障不再判红** ⇒ ② 的红确来自比对）"
        else
            bad "负控 C 未生效（自伤档仍判红）"
        fi
    fi
    restore_all
    cmp -s "$SNAP/devbuild.sh.snap" "$D" || bad "负控 C 还原后 devbuild.sh 不一致"
else
    note "（--neg-skip：跳过负控 A/B/C）"
fi

# ───────────────────────── ⑦ 覆盖边界 ─────────────────────────
hdr "[7/7] 覆盖边界登记（如实）"
note "① 本次只覆盖 **pxc 件**；pxi/pxl/pxfmt 等共用同一条 DEVB_KEY ⇒ 同一判据（未逐件重跑省时）。"
note "② **CI 的冷 .rtcache**（从零现编、.complete 由 rt_cache_compile 落）未在本机复现 ——"
note "   本机 .rtcache 已暖（.complete 全在）⇒ 第 ④ 层的回退是**注入**出来的。"
note "③ 归因 diff 只展示前 8 行（head -8）：大面积改动会截断，但**必然含首个变化的文件**。"
note "④ 三方资产用「名字+大小」判 ⇒ **同大小换内容**（如重编译出等长的 .a）不会被发现；"
note "   这类件是预置、跨轮不变的（真变了会走发布链核对），且省去 14MB 全量 sha256。"
note "⑤ 「key 为什么漂移」的**跨轮**归因仍要靠 CI 台账注解：本轮已让它带上 key 取值与来源列。"
note "⑥ 本门结束前 restore_all ⇒ 源码内容与 mtime 一并回到基线；若中途被 SIGKILL，"
note "   EXIT trap 兜底（M220 记过的「中断后先证明工作树 == 预期版本」）。"

echo
echo "   ⏱ 本门耗时 $((SECONDS-MAX0))s"
if [ "$fail" = "0" ]; then
    echo "M223-VERIFY-OK（全部门通过：$pass）"
else
    echo "M223-VERIFY-FAIL（失败 $fail 项）"
fi
exit "$fail"
