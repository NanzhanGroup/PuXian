#!/usr/bin/env bash
# ============================================================
# M253 门 · **10 个「从未被任何门语料触碰过」的 [B 类] native**
# ------------------------------------------------------------
# 来历：M250 的覆盖面台账（selfhost/check_native_coverage.py）把 392 个 native 逐条过了一遍，
#   留下 19 条「从没有任何普贤语料调用过」；其中 10 条是 **真·用户面 API**（[B 类·欠账]）：
#     aes_encrypt_bytes / aes_decrypt_bytes / aes_gcm_encrypt_bytes / aes_gcm_decrypt_bytes
#     aes_decrypt_ecb / go_errno_string / print_err / session_id / session_destroy / tz_local
#   —— 这正是 M241「能力存在、但从没被证明可用」的形状（那次是发布包里整族被裁掉）。
#
# 本轮把它们全部**补上语料**，并当场照出两个产品缺陷（都是「三轨一致地错」⇒ 只有期望值层看得见）：
#   · 缺陷 A：AES-GCM 解密拒绝**空明文**（16 字节 = 纯 tag）
#             `if (ctlen < 17)` / `if (alllen < 17)` 把「至少 1 字节密文」当成了不变量，
#             而 GCM 的密文长度域是 [0,∞) ⇒ **自己加密的 16 字节自己解不开**，
#             且与 Go crypto/aes-gcm 在空明文上不互通（aead.Seal(pt=[]) 恰好 16 字节）。
#   · 缺陷 B：`go_errno_string` 的 `(int)` 截断（静默错值）
#             2^32+1 → 截成 1 → 回「operation not permitted」（用户从未问过的 errno）；
#             2^31 → 截成负数 → 回「errno -2147483648」。与 M250 缺陷 433 同族。
#   附带更正：速查表把 `aes_encrypt` 参数序写作 `(key, iv, data)` —— 实现与错误消息都是
#             `(data, key, iv)`（本门用 Go 真值把这条口径**钉死**）。
#
# 判据：
#   [1] 静态：生成器自证（10 个 API 必须真的出现在语料里 + 规模锚点）
#   [2] 三轨对拍：87 例 × 3 轨 ⇒ **分叉 0**
#   [3] 期望值：55 例必须逐字节等于 **Go 独立算出**的真值（AES 向量 + errno 全表 0..140 + 越界）
#   [4] 边界面：32 例逐位置 × 错类型 / 错 arity ⇒ 必须**响亮**（rc≠0 + R1002）
#   [5] 负控 A：忠实撤回缺陷 A 的修复 ⇒ 期望值层必须红（r_gcm_* 族）
#   [6] 负控 B：忠实撤回缺陷 B 的修复 ⇒ 期望值层必须红（v_errno_big* 族）
#   [7] 负控 C：判据自伤（--harm）⇒ A 的红**消失**（证明红来自比对本身）
#   [8] 覆盖边界（如实登记）
# CI 用 `--neg-skip`（负控要重编两轨驱动器 + 重编解释轨件）。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m253_bcorpus
W=${M253_W:-/tmp/m253_gate}
rm -rf "$W"; mkdir -p "$W"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }

RT="$ROOT/runtime/runtime.c"
RA="$ROOT/runtime/runtime_aes.c"
snap()    { python3 "$D/negctl.py" --root "$ROOT" --work "$W" --save > "$W/snap.log" 2>&1; }
restore() { python3 "$D/negctl.py" --root "$ROOT" --work "$W" --restore >> "$W/snap.log" 2>&1; }
trap 'restore' EXIT
snap

# 解释轨件：默认用**入库件**（重烘后已含本轮修复）；可用 M253_PXI 覆盖（负控用开发件）
PXI="${M253_PXI:-$ROOT/bootstrap/pxi}"

echo "=== [1] 静态：语料生成（10 个 [B 类] API 必须都在）"
python3 "$D/gen_cases.py" --out "$W" > "$W/gen.log" 2>&1
genrc=$?
cat "$W/gen.log" | sed 's/^/  /'
chk "[1] 生成 rc=0" "[ $genrc -eq 0 ]"
chk "[1] 10 个 API 缺席 0" "grep -q '缺席 0' $W/gen.log"
chk "[1] 规模锚点：例数 ≥ 80" "grep -qE '例 [89][0-9]|例 [1-9][0-9][0-9]' $W/gen.log"
chk "[1] 规模锚点：值例 ≥ 50" "grep -qE '值 [5-9][0-9]|值 [1-9][0-9][0-9]' $W/gen.log"
chk "[1] 负控锚点自证（4 处，逐条唯一）" "python3 $D/negctl.py --root $ROOT --work $W --selftest | grep -q '自证 OK'"
# 覆盖面**冒烟件（入库）**：台账只扫 `.px`，而门的语料是运行期生成的 ⇒ 必须另有入库冒烟。
timeout 60 "$PXI" "$D/surface.px" > "$W/surface.log" 2>&1
chk "[1] 入库冒烟件 surface.px ⇒ M253-SURFACE-OK" "grep -q '^M253-SURFACE-OK$' $W/surface.log"
miss10=0
for n in aes_encrypt_bytes aes_decrypt_bytes aes_gcm_encrypt_bytes aes_gcm_decrypt_bytes aes_decrypt_ecb go_errno_string print_err session_id session_destroy tz_local; do
    grep -q "$n" "$D/surface.px" || miss10=$((miss10+1))
done
chk "[1] surface.px 覆盖 10 个 API（台账消费面）" "[ $miss10 -eq 0 ]"
[ $genrc -ne 0 ] && { echo "生成失败，后续层跳过"; echo "M253-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; }

build_drivers() {   # 建 VM / C 两轨驱动器 → $W/build/{drv_vm,drv_c}
    rm -rf "$W/a_vm" "$W/a_c" "$W/build"
    mkdir -p "$W/a_vm" "$W/a_c" "$W/build"
    cp "$W/drv.px" "$W/a_vm/drv.px"; cp "$W/drv.px" "$W/a_c/drv.px"
    ( cd "$W/a_vm" && timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_vm.log" 2>&1 || { tail -12 "$W/b_vm.log"; return 1; }
    ( cd "$W/a_c"  && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_c.log" 2>&1 || { tail -12 "$W/b_c.log"; return 1; }
    cp -f "$W/a_vm/build/drv" "$W/build/drv_vm" && cp -f "$W/a_c/build/drv" "$W/build/drv_c"
    [ -x "$W/build/drv_vm" ] && [ -x "$W/build/drv_c" ]
}

x3() { local px="${2:-$PXI}"; python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --pxi "$px" --json "$W/rows.json" > "$1" 2>&1; return $?; }
# 从日志里抽计数（判据贴着**现象**写）
nof() { grep -m1 '期望不符' "$1" | sed -n 's/.*期望不符 \([0-9]*\) .*/\1/p'; }
ndv() { grep -m1 '三轨分叉' "$1" | sed -n 's/.*三轨分叉 \([0-9]*\) .*/\1/p'; }

echo "=== [2][3][4] 动态：两轨驱动器 + 三轨 + 期望值 + 边界面（87 例）"
if ! build_drivers; then
    chk "[2] 两轨驱动器构建" "false"
else
    chk "[2] 两轨驱动器构建" "true"
    x3 "$W/base.log"; rc=$?
    cat "$W/base.log" | sed 's/^/  /'
    chk "[2] 三轨对拍 rc=0（分叉 0）" "[ $rc -eq 0 ]"
    chk "[2] 覆盖规模 ≥ 87 例 × 3 轨" "grep -qE '：([89][0-9]|[1-9][0-9][0-9]) 例 × 3 轨' $W/base.log"
    chk "[3] 期望值全符（55 例）" "[ \"\$(nof $W/base.log)\" = \"0\" ]"
    chk "[4] 边界面全响亮（32 例）" "grep -q '边界面未响亮 0' $W/base.log"
    # 留基线件：负控会重编驱动器覆盖 $W/build
    cp -f "$W/build/drv_vm" "$W/drv_vm.base"; cp -f "$W/build/drv_c" "$W/drv_c.base"
fi

if [ "$NEG" = "1" ]; then
    for NC in A B; do
        if [ "$NC" = "A" ]; then
            echo "=== [5] 负控 A：忠实撤回缺陷 A 的修复（AES-GCM 空明文）"
            KEYFAM='r_gcm_b_k16_p0_rt'; MEMO='GCM 空明文往返'
        else
            echo "=== [6] 负控 B：忠实撤回缺陷 B 的修复（go_errno_string int 截断）"
            KEYFAM='v_errno_big32p1'; MEMO='errno 越界'
        fi
        python3 "$D/negctl.py" --root "$ROOT" --work "$W" --patch $NC > "$W/nc$NC.patch.log" 2>&1 || { chk "[$NC] 打桩" "false"; continue; }
        chk "[$NC] 打桩（每处命中 1 次）" "grep -q '已撤回' $W/nc$NC.patch.log"
        if build_drivers; then
            chk "[$NC] 驱动重建" "true"
            # ⭐ 解释轨件也必须在**同一份撤回后的 runtime** 上重建 ——
            #    否则三轨不同源（入库 pxi 仍含修复）⇒ 只看到「解释轨 ⇄ 编译轨」的分叉，
            #    而**测不到**本门最想要的那条证据：「**三轨一致地错**」。
            NCPXI=""
            if ./selfhost/devbuild.sh pxi > "$W/dev_xi_$NC.log" 2>&1 && [ -x /tmp/pxidev ]; then
                cp -f /tmp/pxidev "$W/pxidev_$NC"; NCPXI="$W/pxidev_$NC"
            fi
            chk "[$NC] 解释轨件同源重建（devbuild pxi）" "[ -n \"$NCPXI\" ]"
            x3 "$W/nc$NC.log" "$NCPXI"
            cat "$W/nc$NC.log" | sed 's/^/  /'
            n=$(nof "$W/nc$NC.log"); f=$(ndv "$W/nc$NC.log")
            chk "[$NC] 期望值层判红（不符 > 0）" "[ \"\${n:-0}\" -gt 0 ]"
            chk "[$NC] 指名到本族（$MEMO）" "grep -q '$KEYFAM' $W/nc$NC.log"
            chk "[$NC] 三轨层**不**判红（三轨一致地错 ⇒ 期望值层不可省）" "[ \"\${f:-1}\" = \"0\" ]"
            if [ "$NC" = "A" ]; then
                echo "=== [7] 负控 C：判据自伤（--harm）⇒ A 的红必须消失"
                python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --pxi "$NCPXI" --harm > "$W/ncC.log" 2>&1
                chk "[7] 自伤后期望不符=0（红确实来自比对）" "grep -q '期望不符 0' $W/ncC.log"
            fi
        else
            chk "[$NC] 驱动重建" "false"
        fi
        restore
        chk "[$NC] 源码逐字节还原" "python3 $D/negctl.py --root $ROOT --work $W --selftest | grep -q '自证 OK'"
        # 回到基线驱动器（隔离 A/B 两个变量）
        [ -x "$W/drv_vm.base" ] && cp -f "$W/drv_vm.base" "$W/build/drv_vm" && cp -f "$W/drv_c.base" "$W/build/drv_c"
    done
else
    echo "=== [5][6][7] 负控（--neg-skip：需要重编两轨驱动器）"
fi

echo "=== [8] 覆盖边界（如实登记）"
cat <<'TXT' | sed 's/^/  /'
  · 只判「rc + R 码 + 消息体 + 程序输出」—— 通道与行:列前缀**不判**（属已登记缺陷 186 族）。
  · 本门的动态面**只有一份实现**（runtime 的 C 函数）⇒ 三轨**必然**一致；
    真正有牙的是 [3] 期望值层与 [4] 边界面 —— [2] 三轨层在本门**只防回归**（三轨真分叉）。
  · AES 面只覆盖 CBC/GCM/ECB 的**加解密与往返**；AAD 非空、tag 长度非 16、IV 非 12 字节
    （GCM 允许）未纳入。
  · `print_err` 只判「落到 stderr + 返回值 null + 多参空格连接」；
    「stdout 先刷」的**时序**（缺陷 186 的修复件）未判 —— 需要 stdout 重定向到管道才可观测。
  · `session_id` / `session_destroy` 只覆盖**无会话上下文**（null / false）；
    真会话路径需要 http_serve 请求上下文（m27 门覆盖）。
  · `tz_local` 只判取值范围（±14h）+ UTC/Asia-Shanghai 真值由 verify 环境决定 ⇒
    本门不固定 TZ（避免 CI 与本机时区差异造成假红）。
TXT

echo "──────────────────────────────────────────"
if [ $fail -eq 0 ]; then echo "M253-VERIFY-OK pass=$pass fail=$fail"; else echo "M253-VERIFY-FAIL pass=$pass fail=$fail"; fi
[ $fail -eq 0 ]
