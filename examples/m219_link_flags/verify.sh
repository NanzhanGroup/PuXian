#!/usr/bin/env bash
# ============================================================
# M219 门 —— 「默认 PIE 工具链」环境依赖收口（缺陷 311/312）
#
# 被验证的对象：
#   · selfhost/sim_pie_cc.sh        —— 「默认 PIE」工具链垫片（把 CI-only 故障变本机可复现）
#   · selfhost/check_link_flags.sh  —— 链接 flag 卫生守卫（静态扫全仓，防同类复发）
#   · 被修的三处链接（selfhost/）：devbuild.sh 的 VM 轨 + bootstrap_prove_bc.sh 的两处
#
# 层：
#   ① 工具自证（垫片 7 判据 · 守卫 5 判据）
#   ② 垫片复现：从**脚本源码**抽出链接 flag 段，用垫片链接「保证带非 PIC 绝对重定位」的探针
#      对象 ⇒ 必须成功；同一探针在**去掉 -static 的同一 flag 串**下必须失败（判据自证）
#   ③ 全仓扫描：违例 0 + 规模下限 + 豁免计数（防豁免表被悄悄清空）
#   ④ 负控 3 道（各自独立判红；源逐字节还原）
#   ⑤ 覆盖边界（如实登记）
#
# 用法：bash examples/m219_link_flags/verify.sh [--neg-skip]
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
NEG_SKIP=0
for a in "$@"; do case "$a" in --neg-skip) NEG_SKIP=1 ;; esac; done

W="$(mktemp -d "${TMPDIR:-/tmp}/m219_gate.XXXXXX")" || exit 2
PASS=0; FAIL=0
ok()  { echo "   ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "   ❌ $1"; FAIL=$((FAIL+1)); }
note(){ echo "   ▸ $1"; }

# ---- 源快照/还原（负控用；逐字节还原 + 自检）----
FILES=("selfhost/devbuild.sh" "selfhost/bootstrap_prove_bc.sh" "selfhost/check_link_flags.sh")
_backup() { local f; for f in "${FILES[@]}"; do cp -f "$ROOT/$f" "$W/$(basename "$f").orig"; done; }
_restore() {
    local f b
    for f in "${FILES[@]}"; do
        b="$W/$(basename "$f").orig"
        [ -f "$b" ] || continue
        cp -f "$b" "$ROOT/$f"
    done
}
_verify_restored() {
    local f b n=0
    for f in "${FILES[@]}"; do
        b="$W/$(basename "$f").orig"
        cmp -s "$b" "$ROOT/$f" || { bad "负控后 $f 未逐字节还原"; n=1; }
    done
    [ "$n" = 0 ] && ok "负控后 3 个源文件逐字节还原"
}
trap '_restore; rm -rf "$W"' EXIT

echo "══ M219 门 · 默认 PIE 工具链环境依赖收口 ══"
echo "── 工作目录 $W（--neg-skip=${NEG_SKIP}）"

# ================= 第 ① 层：工具自证 =================
echo "── [1/5] 工具自证（**两档宿主**）"
if bash "$ROOT/selfhost/sim_pie_cc.sh" --self-test > "$W/sim.log" 2>&1; then
    if grep -q "通过 7 · 失败 0" "$W/sim.log"; then ok "垫片自证 7/0（S1–S6 + N1 · 本机宿主）"; else bad "垫片自证计数不符"; tail -8 "$W/sim.log"; fi
else
    bad "垫片自证 rc≠0"; tail -12 "$W/sim.log"
fi
# ①b **跨宿主**自证：在「默认 PIE 宿主」模拟下再跑一遍
#   为什么：M219s1 的 N1 首版断言「不补 -pie ⇒ 必须成功」—— 只在**非 PIE 默认**宿主成立，
#   而 ubuntu-latest（gcc 13.3）自身就加 -pie ⇒ CI 必红（实测注解 `N1 … (r1=1 rn=1)`）。
#   本机是 Red Hat 系 ⇒ 不把「另一种宿主」搬回本机，这类回归就无人拦。
SIMA="$(PX_PIE_SHIM_DIR="$W/simshim" bash "$ROOT/selfhost/sim_pie_cc.sh" --make)"
if PX_REAL_CC="$SIMA/gcc" PX_PIE_SHIM_DIR="$W/nested" \
   bash "$ROOT/selfhost/sim_pie_cc.sh" --self-test > "$W/sim2.log" 2>&1; then
    if grep -q "通过 7 · 失败 0" "$W/sim2.log"; then
        ok "垫片自证 7/0 **在「默认 PIE 宿主」模拟下同样成立**（跨宿主口径）"
    else bad "PIE 宿主模拟下垫片自证计数不符"; tail -8 "$W/sim2.log"; fi
else
    bad "PIE 宿主模拟下垫片自证 rc≠0 ⇒ 存在**宿主相关判据**"; tail -12 "$W/sim2.log"
fi
# 模拟必须**确认生效**（首版用 SHIM_DIR_DEFAULT 同名目录 ⇒ 被 resolve_real_cc 跳过 ⇒ 静默没生效）
grep -q "宿主默认 PIE" "$W/sim2.log" && ok "模拟确已生效（日志出现「宿主默认 PIE」）" \
    || bad "模拟未生效（垫片没把模拟宿主当成真实 cc）"
if bash "$ROOT/selfhost/check_link_flags.sh" --self-test > "$W/clf.log" 2>&1; then
    if grep -q "通过 5 · 失败 0" "$W/clf.log"; then ok "守卫自证 5/0（F1–F4 + N1）"; else bad "守卫自证计数不符"; tail -8 "$W/clf.log"; fi
else
    bad "守卫自证 rc≠0"; tail -12 "$W/clf.log"
fi

# ================= 第 ② 层：垫片复现 =================
echo "── [2/5] 垫片复现（从脚本源码抽链接 flag ⇒ 非 PIC 探针必须能链/不能链）"
bash "$ROOT/selfhost/sim_pie_cc.sh" --info > "$W/info.log" 2>&1 || true
sed 's/^/   ▸ /' "$W/info.log"
SHIM="$(bash "$ROOT/selfhost/sim_pie_cc.sh" --make)" || { bad "垫片目录创建失败"; exit 1; }
REAL_CC="$(PATH="$(dirname "$SHIM")" command -v gcc 2>/dev/null || command -v gcc)"
# 非 PIC 探针对象（`-fno-pic` + 运行期下标的 rodata 取址 ⇒ 绝对重定位）
cat > "$W/np.c" <<'EOF'
static const char px_pie_probe_msg[] = "px-pie-probe-0123456789";
int px_pie_probe_get(int i) { return px_pie_probe_msg[i & 15]; }
EOF
printf 'int main(void){return 0;}\n' > "$W/m.c"
"$REAL_CC" -c -fno-pic -O2 -o "$W/np.o" "$W/np.c" || bad "非 PIC 探针对象编译失败"
"$REAL_CC" -c -O2 -o "$W/m.o" "$W/m.c" || bad "主对象编译失败"
if readelf -r "$W/np.o" 2>/dev/null | grep -qE 'R_X86_64_32(S)?|R_AARCH64_(ABS|ADR_PREL_LO21)'; then
    ok "非 PIC 探针对象带绝对重定位（判据有牙的前提）"
else
    bad "非 PIC 探针对象未带绝对重定位 —— 本层失去区分力"
fi

# 抽出「编译器 token 与 -o 之间」的 flag 段
#   ⚠️ **关键字必须锚在链接行上**（含 `-o <产物>`）：首版用 `compiler_new` 当关键字 ⇒
#      命中的是 `gcc -c … -o /tmp/bpbc_cn.o` 那条**编译行** ⇒ 抽出 `-c …` ⇒ 三道判据全假红。
extract_flags() {   # $1=文件 $2=链接行关键字（含 -o <产物>）⇒ 打印 flag 段（抽不到则空）
    local f="$1" key="$2" ln raw
    ln="$(grep -nF -- "$key" "$ROOT/$f" | head -1 | cut -d: -f1)"
    [ -n "$ln" ] || return 1
    raw="$(sed -n "${ln}p" "$ROOT/$f")"
    case "$raw" in *' -c '*) return 1 ;; esac      # 抽到编译行 ⇒ 判定不了不许放行
    printf '%s' "$raw" | sed -E 's/^[[:space:]]*[^[:space:]]+[[:space:]]+//; s/[[:space:]]+-o[[:space:]].*$//'
}
SITES=("selfhost/devbuild.sh|-o /tmp/pxcdev_vm"
       "selfhost/bootstrap_prove_bc.sh|-o \"\$BUILD/compiler_new\""
       "selfhost/bootstrap_prove_bc.sh|-o \"\$BUILD/compiler_vm\"")

# flag 段自检：每个 token 都必须以 `-` 开头
#   ⚠️ 首版 NC-A 用 `pxcdev_vm` 当关键字 ⇒ 命中的是**用法注释行**
#      （`#   ./selfhost/devbuild.sh --vm  …/tmp/pxcdev_vm`）⇒ 抽出的「flag」里混进
#      `./selfhost/devbuild.sh` / `--vm` ⇒ 探针因 `unrecognized option '--vm'` 而失败
#      ⇒ **判据绿灯但理由完全错**（判据假绿）。⇒ 非 flag token 一律判红。
_flags_ok() {
    local t
    [ -n "$1" ] || return 1
    for t in $1; do case "$t" in -*) : ;; *) return 1 ;; esac; done
    return 0
}

for s in "${SITES[@]}"; do
    f="${s%%|*}"; key="${s#*|}"
    fl="$(extract_flags "$f" "$key")" || { bad "$f（$key）抽不到链接 flag 段 —— 判定不了不许放行"; continue; }
    _flags_ok "$fl" || { bad "$f（$key）抽到的不是 flag 段（含非 - 开头 token）：[$fl]"; continue; }
    case " $fl " in
        *" -static "*|*" -no-pie "*|*" -pie "*) : ;;
        *) bad "$f（$key）flag 段缺 -static/-no-pie：[$fl]"; continue ;;
    esac
    # 垫片环境下必须**链接成功**
    if PATH="$SHIM:$PATH" gcc $fl -o "$W/out1" "$W/m.o" "$W/np.o" >"$W/l1.log" 2>&1; then
        ok "$f（$key）垫片下链接成功（flag=[$fl]）"
    else
        bad "$f（$key）垫片下链接失败：$(head -1 "$W/l1.log" | cut -c1-90)"
    fi
    # 同一 flag 串**去掉 -static** ⇒ 必须失败（判据自证：探针 + 垫片都有牙）
    fl2="$(printf '%s' "$fl" | sed 's/ *-static//')"
    if PATH="$SHIM:$PATH" gcc $fl2 -o "$W/out2" "$W/m.o" "$W/np.o" >"$W/l2.log" 2>&1; then
        bad "$f（$key）去掉 -static 后**仍成功** ⇒ 探针/垫片无区分力"
    else
        ok "$f（$key）去掉 -static 即失败（故障面确实是 PIE 口径）"
    fi
done

# ================= 第 ③ 层：全仓扫描 =================
echo "── [3/5] 全仓链接 flag 卫生扫描"
bash "$ROOT/selfhost/check_link_flags.sh" "$ROOT" > "$W/scan.log" 2>&1; rc=$?
sed 's/^/   ▸ /' "$W/scan.log"
nf="$(sed -n 's/.*命中文件 \([0-9]*\).*/\1/p' "$W/scan.log" | head -1)"
nl="$(sed -n 's/.*链接命令 \([0-9]*\) 条.*/\1/p' "$W/scan.log" | head -1)"
exv="$(sed -n 's/.*（变量 \([0-9]*\) \/ 行级 \([0-9]*\)）.*/\1/p' "$W/scan.log" | head -1)"
exl="$(sed -n 's/.*（变量 \([0-9]*\) \/ 行级 \([0-9]*\)）.*/\2/p' "$W/scan.log" | head -1)"
exn="$(sed -n 's/.*豁免 \([0-9]*\)（.*/\1/p' "$W/scan.log" | head -1)"
[ "$rc" = 0 ] && ok "扫描 rc=0" || bad "扫描 rc=$rc（有违例）"
[ "${nf:-0}" -ge 250 ] && ok "扫描面规模：文件 $nf（下限 250）" || bad "扫描面过小：文件 ${nf:-0} < 250"
[ "${nl:-0}" -ge 18 ] && ok "链接命令 $nl 条（下限 18）" || bad "链接命令过少：${nl:-0} < 18"
[ "${exn:-0}" = "$(( ${exv:-0} + ${exl:-0} ))" ] && ok "豁免总数 $exn == 变量 $exv + 行级 $exl" || bad "豁免计数自相矛盾：$exn != $(( ${exv:-0} + ${exl:-0} ))"
[ "${exv:-0}" = 5 ] && ok "变量豁免 5 条（口径参数化的两处 + 本门探针 $fl）" || bad "变量豁免计数 ${exv:-0} ≠ 5"
[ "${exl:-0}" = 3 ] && ok "行级豁免 3 条（含**过期判据**兜底）" || bad "行级豁免计数 ${exl:-0} ≠ 3"
grep -q '违例 0 条' "$W/scan.log" && ok "违例 0 条" || bad "扫描报出违例"

# ================= 第 ④ 层：负控 =================
if [ "$NEG_SKIP" = 1 ]; then
    echo "── [4/5] 负控：--neg-skip，跳过"
else
    echo "── [4/5] 负控（3 道，各自独立判红）"
    _backup

    # NC-A：撤 devbuild.sh 的 -static ⇒ ① 守卫必报违例 ② 探针必**以 PIE 类错误**失败
    sed -i 's|gcc -static -O2 -pthread -o /tmp/pxcdev_vm|gcc -O2 -pthread -o /tmp/pxcdev_vm|' "$ROOT/selfhost/devbuild.sh"
    if grep -q '^    gcc -O2 -pthread -o /tmp/pxcdev_vm' "$ROOT/selfhost/devbuild.sh"; then
        fl="$(extract_flags "selfhost/devbuild.sh" '-o /tmp/pxcdev_vm')"
        if ! _flags_ok "$fl"; then
            bad "NC-A 抽错行（不是 flag 段）：[$fl]"
        elif PATH="$SHIM:$PATH" gcc $fl -o "$W/nca" "$W/m.o" "$W/np.o" >"$W/nca.log" 2>&1; then
            bad "NC-A 探针：撤 -static 后仍链接成功"
        elif grep -q "PIE" "$W/nca.log"; then
            ok "NC-A 探针：撤 -static ⇒ 垫片下 **PIE 类**失败（$(head -1 "$W/nca.log" | cut -c1-64)…）"
        else
            bad "NC-A 探针：虽失败但**不是 PIE 类**（判据失去区分力）：$(head -1 "$W/nca.log" | cut -c1-80)"
        fi
        bash "$ROOT/selfhost/check_link_flags.sh" "$ROOT" > "$W/nca.scan" 2>&1 && bad "NC-A 守卫：撤 -static 后仍 rc=0" \
            || { { grep -qE '^   ❌ selfhost/devbuild.sh:' "$W/nca.scan" && grep -q 'pxcdev_vm' "$W/nca.scan"; } \
                 && ok "NC-A 守卫：点出 devbuild.sh 的 VM 轨链接（不按行号，防注释漂移）" \
                 || bad "NC-A 守卫：未点出 devbuild.sh 的 VM 轨链接"; }
    else
        bad "NC-A 注入失败（锚点不匹配）"
    fi
    _restore

    # NC-B：撤 bootstrap_prove_bc.sh 的一处 -static ⇒ 守卫必报
    sed -i 's|gcc -static -O2 -pthread -o "\$BUILD/compiler_new"|gcc -O2 -pthread -o "$BUILD/compiler_new"|' "$ROOT/selfhost/bootstrap_prove_bc.sh"
    if grep -q '^    gcc -O2 -pthread -o "\$BUILD/compiler_new"' "$ROOT/selfhost/bootstrap_prove_bc.sh"; then
        bash "$ROOT/selfhost/check_link_flags.sh" "$ROOT" > "$W/ncb.scan" 2>&1 && bad "NC-B 守卫：撤 -static 后仍 rc=0" \
            || { { grep -qE '^   ❌ selfhost/bootstrap_prove_bc.sh:' "$W/ncb.scan" && grep -q 'compiler_new' "$W/ncb.scan"; } \
                 && ok "NC-B 守卫：点出 bootstrap_prove_bc.sh 的 compiler_new 链接" \
                 || bad "NC-B 守卫：未点出该链接"; }
    else
        bad "NC-B 注入失败（锚点不匹配）"
    fi
    _restore

    # NC-C：判据自伤（合规正则恒真）⇒ 守卫**自证**的 F1 必须判红
    sed -i "s|^COMPLIANT_RX=.*|COMPLIANT_RX='.'|" "$ROOT/selfhost/check_link_flags.sh"
    if grep -q "^COMPLIANT_RX='\.'" "$ROOT/selfhost/check_link_flags.sh"; then
        bash "$ROOT/selfhost/check_link_flags.sh" --self-test > "$W/ncc.log" 2>&1 \
            && bad "NC-C 判据自伤：自证仍绿（判据无自省力）" \
            || { grep -q 'F1' "$W/ncc.log" && ok "NC-C 判据自伤：自证 F1 判红（判据确有牙）" || bad "NC-C 判据自伤：自证虽红但非 F1"; }
    else
        bad "NC-C 注入失败（锚点 COMPLIANT_RX 不匹配）"
    fi
    _restore
    _verify_restored
fi

# ================= 第 ⑤ 层：覆盖边界 =================
echo "── [5/5] 覆盖边界（如实登记）"
note '**已覆盖**：① 垫片语义（7 判据，与宿主默认值无关）② 三个 selfhost 站点的链接 flag 真值'
note "   （从**脚本源码**抽出后真的链接，非文本匹配；抽到编译行即判红）③ 全仓扫描：文件 $nf · 链接命令 $nl 条"
note '  ④ 负控 3 道（撤 flag ×2 + 判据自伤）⑤ 豁免表**过期判据**（行级豁免失效即判红）'
note '**未覆盖**：① 完整 devbuild（pxc --vm）端到端（需 .rtcache ⇒ 环境依赖）—— 本门用'
note '   「真抽 flag + 真链接」替代；端到端由本地全量门 m116 与 CI 质量门覆盖'
note '  ② 非 selfhost 的 4 处站点（m89_s3d / m90_s1 / m89_a2 / m217 ub_proof）只由静态守卫覆盖'
note '   （探针式需从源码抽 flag，而那几行形态各异 ⇒ 不做，避免「抽不到就静默放过」）'
note '  ③ aarch64 宿主的**行为**（本机无 qemu）—— 探针对象在 aarch64 上也应产生绝对重定位，'
note '   判据由 S1/S2 的构造保证；若某宿主上不再有牙，第 ② 层会**响亮判红**而非静默通过'
note '  ④ 动态库（.so）链接 —— 本仓口径是 -static 产物，.so 不在判据面内'

echo "── 小计：通过 $PASS · 失败 $FAIL"
if [ "$FAIL" = 0 ]; then echo "M219-VERIFY-OK"; exit 0; else echo "M219-VERIFY-FAIL"; exit 1; fi
