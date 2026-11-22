#!/usr/bin/env bash
# ============================================================
# selfhost/check_pxrun_stdin.sh —— px run 解释轨的 **stdin 继承**（M436 · 缺陷 905）
# ------------------------------------------------------------
# 病史（晨曦 CX 报 2026-11-23 01:35 · 本地复现 03:5x）：
#   printf 'hello=world' | px run t.px        ⇒ len=0    ❌
#   printf 'hello=world' | px run --fast t.px ⇒ len=11   ✅
#   printf 'hello=world' | bootstrap/pxi t.px ⇒ len=11   ✅
#   病因：M429（缺陷 878 · 防「杀父留孙」）把前台直调改成 `"$PXI_BIN" … &`。
#     bash 在**非交互**模式下对异步命令默认把 fd0 接到 /dev/null（无 job control），
#     ⇒ pxi 读到空 stdin，管道/重定向内容被**静默丢弃**（不报错、不阻塞）。
#   修法：`0<&0` 显式把调用方 stdin 复制给子进程（不动 trap / wait —— M429 的语义不变）。
#
# 判据（每条都能判红）：
#   S0  环境守卫：tools/px · bootstrap/pxi · timeout 缺失 ⇒ rc=3（绝不降级成 0）
#   S1  正控·小输入（11 字节）：三轨 len 全 = 11 且**逐轨相等**
#   S2  大输入 1 MiB：interp / fast 都 = 1048576（管道单次 read ≤ 64KiB ⇒ 顺带验分块读）
#   S3  /dev/null：len=0 且 timeout 内退出（**不得**阻塞 —— 修前若把 /dev/null 复制错会挂）
#   S4  空管道：len=0
#   S5  文件重定向：len=11（覆盖「非管道」的 stdin 形态）
#   S6  无污染：interp 与 fast 的 **stdout 逐字节相同**（提示行走 stderr，不得混进 stdout）
#   N1  负控：把 ` 0<&0` 从 **tools/px** 删掉 ⇒ interp 轨必须回 len=0，且 fast 轨仍 = 11
#       （证明红来自这条 `&` 的 stdin，不是脚本/环境；也证明判据有牙）
#   N1b 前提自证：负控补丁必须真落地（改后 grep 不到 `0<&0`）且跑之前正控已绿
#   G1  零污染：tools/px 的 sha256 与 git status 行数前后一致（跑完必须原样）
#   G2  M429 形状仍在（静态）：4 条 trap + wait 循环 + KILL 兜底 —— 本轮改动不得把它弄丢
#
# 用法：check_pxrun_stdin.sh              # 全跑（约 20~60s，含一次负控）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
PXI="$ROOT/bootstrap/pxi"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m436_stdin.XXXXXX")"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }

# ── S0 环境守卫 ───────────────────────────────────────────
if [ ! -x "$PX" ]; then echo "M436-STDIN-ENV-MISSING: $PX"; exit 3; fi
if [ ! -x "$PXI" ]; then echo "M436-STDIN-ENV-MISSING: $PXI"; exit 3; fi
command -v timeout >/dev/null 2>&1 || { echo "M436-STDIN-ENV-MISSING: timeout"; exit 3; }

SRC="$TMP/len.px"
cat > "$SRC" <<'PXEOF'
total = 0
go = true
while go:
    b = read(0, 65536)
    if len(b) == 0:
        go = false
    else:
        total = total + len(b)
print("len=" + str(total))
PXEOF

# 读数：$1 = 轨道名，stdin 由调用方接
len_of() {
    local tr="$1"
    case "$tr" in
        interp) timeout 120 "$PX" run "$SRC" 2>/dev/null ;;
        fast)   timeout 120 "$PX" run --fast "$SRC" 2>/dev/null ;;
        pxi)    timeout 120 "$PXI" "$SRC" 2>/dev/null ;;
    esac | tr -d '\r' | sed -n 's/^len=\([0-9][0-9]*\)$/\1/p' | tail -1
}

# ── S1 正控·小输入 ────────────────────────────────────────
A="$(printf 'hello=world' | len_of interp)"
B="$(printf 'hello=world' | len_of fast)"
C="$(printf 'hello=world' | len_of pxi)"
if [ "$A" = 11 ] && [ "$B" = 11 ] && [ "$C" = 11 ]; then
    ok "S1 小输入 11 字节：三轨 len 全 11（interp/fast/pxi）"
else
    bad "S1 小输入三轨不一致（interp=${A:-?} fast=${B:-?} pxi=${C:-?}，期望全 11）"
fi

# ── S2 大输入 1 MiB ──────────────────────────────────────
D="$(head -c 1048576 /dev/zero | len_of interp)"
E="$(head -c 1048576 /dev/zero | len_of fast)"
if [ "$D" = 1048576 ] && [ "$E" = 1048576 ]; then
    ok "S2 1 MiB 分块读：interp=1048576 · fast=1048576"
else
    bad "S2 1 MiB 读不完整（interp=${D:-?} fast=${E:-?}，期望 1048576）"
fi

# ── S3 /dev/null 不阻塞 ──────────────────────────────────
D0=$(date +%s)
timeout 20 "$PX" run "$SRC" < /dev/null > "$TMP/devnull.out" 2>/dev/null
RC=$?
D1=$(( $(date +%s) - D0 ))
F="$(tr -d '\r' < "$TMP/devnull.out" | sed -n 's/^len=\([0-9][0-9]*\)$/\1/p' | tail -1)"
if [ "$F" = 0 ] && [ "$RC" = 0 ] && [ "$D1" -lt 15 ]; then
    ok "S3 /dev/null：len=0 且 ${D1}s 内退出（不阻塞）"
else
    bad "S3 /dev/null 异常（len=${F:-?} rc=$RC 用时 ${D1}s）"
fi

# ── S4 空管道 ────────────────────────────────────────────
G="$(printf '' | len_of interp)"
[ "$G" = 0 ] && ok "S4 空管道：len=0（不报错）" || bad "S4 空管道异常（len=${G:-?}，期望 0）"

# ── S5 文件重定向 ────────────────────────────────────────
printf 'hello=world' > "$TMP/in.txt"
H="$(timeout 60 "$PX" run "$SRC" < "$TMP/in.txt" 2>/dev/null | tr -d '\r' | sed -n 's/^len=\([0-9]*\)$/\1/p' | tail -1)"
[ "$H" = 11 ] && ok "S5 文件重定向：len=11" || bad "S5 文件重定向异常（len=${H:-?}，期望 11）"

# ── S6 stdout 无污染 ─────────────────────────────────────
printf 'hello=world' | timeout 60 "$PX" run "$SRC" 2>/dev/null > "$TMP/o_interp.txt"
printf 'hello=world' | timeout 60 "$PX" run --fast "$SRC" 2>/dev/null > "$TMP/o_fast.txt"
if [ -s "$TMP/o_interp.txt" ] && cmp -s "$TMP/o_interp.txt" "$TMP/o_fast.txt"; then
    ok "S6 stdout 一致（interp == fast，逐字节）"
else
    bad "S6 stdout 不一致（可能是提示行混进 stdout）"
fi

# ── N1/N1b 负控：删掉 ` 0<&0` ────────────────────────────
PX_BAK="$TMP/px.bak"
SHA0="$(sha256sum "$PX" | cut -d' ' -f1)"
GIT0="$(cd "$ROOT" && git status --porcelain -- tools/px 2>/dev/null | wc -l)"
cp -p "$PX" "$PX_BAK"
PATCHED=0
restore_px() { [ "$PATCHED" = 1 ] && cp -p "$PX_BAK" "$PX" && PATCHED=0; }
trap 'restore_px; rm -rf "$TMP"' EXIT

python3 - "$PX" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '"$PXI_BIN" "$@" "$file" 0<&0 &'
new = '"$PXI_BIN" "$@" "$file" &'
n = s.count(old)
if n != 1:
    print("ANCHOR-MISMATCH %d" % n); sys.exit(3)
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PY
if [ $? -ne 0 ]; then
    bad "N1b 负控补丁未落地（锚点不唯一）"
else
    PATCHED=1
    # 前提自证：补丁确实生效（判据必须看**代码行**，注释里也写着 0<&0 —— 教训：注释里的代码也是代码）
    if grep -q '"\$PXI_BIN" "\$@" "\$file" 0<&0 &' "$PX"; then
        bad "N1b 负控前提不成立（补丁后仍能 grep 到 0<&0）"
    else
        ok "N1b 负控前提自证：补丁后 tools/px 已无 0<&0"
        N1="$(printf 'hello=world' | len_of interp)"
        N1F="$(printf 'hello=world' | len_of fast)"
        if [ "$N1" = 0 ] && [ "$N1F" = 11 ]; then
            ok "N1 负控判红：撤 0<&0 后 interp len=0（fast 仍 11）⇒ 判据有牙"
        else
            bad "N1 负控未判红（interp=${N1:-?} fast=${N1F:-?}，期望 interp=0 / fast=11）"
        fi
    fi
fi
restore_px

# ── G1 零污染 ───────────────────────────────────────────
SHA1="$(sha256sum "$PX" | cut -d' ' -f1)"
GIT1="$(cd "$ROOT" && git status --porcelain -- tools/px 2>/dev/null | wc -l)"
if [ "$SHA0" = "$SHA1" ] && [ "$GIT0" = "$GIT1" ]; then
    ok "G1 tools/px 原样回滚（sha ${SHA0:0:12} · git status $GIT0 → $GIT1）"
else
    bad "G1 tools/px 未回滚（sha ${SHA0:0:12} → ${SHA1:0:12} · git status $GIT0 → $GIT1）"
fi

# ── G2 M429 形状仍在（静态）──────────────────────────────
MISS=""
for pat in 'trap ._px_fwd TERM' 'trap ._px_fwd INT' 'trap ._px_fwd HUP' 'trap ._px_fwd QUIT' \
           'kill -KILL "$_child"' 'wait "$_child"'; do
    grep -q "$pat" "$ROOT/tools/px" || MISS="$MISS [$pat]"
done
if [ -z "$MISS" ]; then
    ok "G2 M429 形状仍在（4 trap + wait 循环 + KILL 兜底）"
else
    bad "G2 M429 形状缺失：$MISS"
fi

echo
echo "=== m436_pxrun_stdin：PASS $PASS / FAIL $FAIL ==="
if [ "$FAIL" = 0 ]; then echo "M436-PXRUN-STDIN-OK"; exit 0; fi
echo "M436-PXRUN-STDIN-FAIL"; exit 1
