#!/usr/bin/env bash
# ============================================================
# M187 门（第 65 轮）：第三方 registry-px 库 **引入官方 registry/** + pxpkg **多文件包**支持
# ------------------------------------------------------------
# 两条主线：
#  ① **引入**：把上游 `banshanhanfu/registry-px`（Apache-2.0）的库**逐字节照搬**进官方
#     `registry/`（**86 包**来自上游 registry-px —— M200 由 53 涨到 86：33 新库 + 8 就地更新；
#      另有 **13 个本仓自建包**（edge/gfx/lunar/pxml/semver/yaml…）不在上游表内；
#      `passhash` 于 **M189** 补齐 —— 因 M184 严格 `int()` 需要一行守卫，
#     当时以**本地补丁**形式引入；**M198 起补丁已撤销**：上游 `7da3397e` 自己加了
#     `is_int_str` 守卫（同一修法），故包内容回到**逐字节照搬**，补丁通道**保留但不使用**）。
#     来源与许可**不改包内注释**，登记在 `registry/THIRD_PARTY.md`（自动生成 + 本门复核）。
#  ② **pxpkg 多文件包**：registry 规范原先只认 `<name>.px` 一个文件，而官方写库规范
#     要求**每文件 <500 行、超了拆**（qrcode 4 文件 / mysql 3 / xlsx 2）⇒ 规范自己跟分发形态冲突。
#     M187 起：包目录下**全部 `.px`** 随包分发（入口恒 `<name>.px`，包内相对 import 原样），
#     digest 兼容（单文件包 = 入口 sha256，与 M45 lock **逐字节兼容**；
#     多文件包 = 各「文件名:内容」串接 sha256 —— 旧格式下不存在多文件包 ⇒ 无历史 lock 受影响）。
#
# 判据（四层正判据 + 三道负控）：
#  ① `registry/THIRD_PARTY.md` 的表**逐行重算 sha256 / 文件数**与磁盘对拍（防漂移）；
#  ② **每个**引入包：`pxpkg add` → `install` → `import <pkg>` 解释轨跑通（86 包全量）；
#  ③ 抽样（含**全部多文件包** + 二进制/解析族）**双轨编译**跑通（VM + C）；
#  ④ 多文件包语义：两文件包三轨跑通 + `--locked` 能查到**辅助文件**被篡改（修前只查入口）。
# 负控（各自独立判红；源逐字节还原）：
#  A 篡改包入口一个字节 ⇒ ① 红（表与磁盘不符）  B 篡改 THIRD_PARTY.md 的 sha ⇒ ① 红
#  C 删多文件包的辅助文件 ⇒ ③ 红（该包编译/导入失败）
#
# 用法：bash examples/m187_registry_import/verify.sh [--neg-skip] [--smoke-limit N]
# 退出码：0 = 绿，1 = 红，2 = 门自身前置自查失败。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
LIMIT=0
for a in "$@"; do
    case "$a" in
        --neg-skip) NEG=0 ;;
        --smoke-limit=*) LIMIT="${a#*=}" ;;
    esac
done

REG="$ROOT/registry"
PROV="$REG/THIRD_PARTY.md"
W=/tmp/m187_gate
BACK=/tmp/m187_gate_bak
rm -rf "$W" "$BACK"; mkdir -p "$W" "$BACK"
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

[ -f "$PROV" ] || { echo "❌ 前置自查失败：缺 registry/THIRD_PARTY.md（先跑 tools/import_registry_px.sh --apply）" >&2; exit 2; }
grep -q 'M187' tools/import_registry_px.sh || { echo "❌ 前置自查失败：引入器缺 M187 标记" >&2; exit 2; }
# M188-STDLIB：本门**必须**开严格模块 —— 否则「stdlib 没找到」只打警告 + 跳过，
#   编译错误被降级成运行期未定义符号，门的 rc 判据会**假绿**（CI 上正是这样被蒙住的）。
export PX_STRICT_MODULE=1
grep -q 'pxp_digest_pkg' tools/pxpkg.px || { echo "❌ 前置自查失败：pxpkg 缺多文件包支持（M187）" >&2; exit 2; }

# ── 表解析：包 / 版本 / 文件数 / 入口 sha 前 16 ──
PKGS=(); VERS=(); NFILES=(); SHAS=()
while IFS='|' read -r _ pkg ver nf sha _rest; do
    pkg="$(echo "$pkg" | tr -d ' ')"; ver="$(echo "$ver" | tr -d ' ')"
    nf="$(echo "$nf" | tr -d ' ')"; sha="$(echo "$sha" | tr -d ' `')"
    case "$pkg" in ''|'包'|'---'*) continue ;; esac
    PKGS+=("$pkg"); VERS+=("$ver"); NFILES+=("$nf"); SHAS+=("$sha")
done < <(grep '^|' "$PROV")
N=${#PKGS[@]}
echo "=== [0] 引入清单：$N 包（来源 registry/THIRD_PARTY.md）"

echo "=== [1] 来源一致性：逐行重算 sha256 / 文件数 与磁盘对拍（防漂移）"
bad=0
for i in $(seq 0 $((N-1))); do
    p="${PKGS[$i]}"; v="${VERS[$i]}"; nf="${NFILES[$i]}"; sh="${SHAS[$i]}"
    d="$REG/$p/$v"
    [ -d "$d" ] || { echo "    缺目录 $p/$v"; bad=$((bad+1)); continue; }
    [ -f "$d/$p.px" ] || { echo "    缺入口 $p/$v/$p.px"; bad=$((bad+1)); continue; }
    actual_n="$(ls "$d"/*.px 2>/dev/null | wc -l)"
    [ "$actual_n" = "$nf" ] || { echo "    文件数不符 $p/$v：表=$nf 实际=$actual_n"; bad=$((bad+1)); }
    actual_sha="$(sha256sum "$d/$p.px" | cut -c1-16)"
    [ "$actual_sha" = "$sh" ] || { echo "    入口 sha 不符 $p/$v：表=$sh 实际=$actual_sha"; bad=$((bad+1)); }
done
chk "[1] $N 包的 sha256/文件数 全部与登记表一致（漂移 0）" "[ $bad = 0 ]"
# M189：passhash 已补齐引入（此前因 M184 严格 int() 而暂缓）。
# M198：上游 `7da3397e` **自修**（`is_int_str` 守卫，与我们的补丁同法）⇒ **撤销本地补丁**，
#   包内容回到逐字节照搬；补丁**通道**保留（M189 能力，用于"上游未修但我方语言面收紧"的场景）。
chk "[1] passhash 已引入（M189）" "grep -qE '^\\| *passhash *\\|' \"$PROV\""
chk "[1] 上游已自修 ⇒ 本地补丁**已撤销**（passhash.patch 不存在）" "[ ! -f tools/patches/registry-px/passhash.patch ]"
chk "[1] 补丁**通道**仍保留（引入器 = M187+M189 补丁逻辑）" "grep -q 'M187' tools/import_registry_px.sh && grep -q 'PATCHDIR=' tools/import_registry_px.sh"
chk "[1] **全部**包的补丁列为空（纪律①：逐字节照搬 —— M198 起无例外）" "[ \"\$(grep -cE '\\|  \\|\$' \"$PROV\")\" = \"$N\" ]"
chk "[1] 多文件包已入库（qrcode=4 · mysql=3 · xlsx=2）" "[ \"\$(ls $REG/qrcode/0.1.0/*.px | wc -l)\" = 4 ] && [ \"\$(ls $REG/mysql/0.1.0/*.px | wc -l)\" = 3 ] && [ \"\$(ls $REG/xlsx/0.1.0/*.px | wc -l)\" = 2 ]"

echo "=== [2] 全量：每包 pxpkg 装 + import 解释轨跑通（$N 包）"
APP="$W/app"; mkdir -p "$APP"; cd "$APP"
export PX_REGISTRY="$REG"
# M188-DIAG：这三步的输出原来被吞掉 / 只打 5 行 ⇒ CI 上根因不可见（job 日志 403、
#   注解只能看到"目录数=0"）。现在**无条件**把关键输出打进 stdout（含 ❌/FAIL 便于注解抓取）。
"$ROOT/tools/pxpkg" init --name m187app > "$W/init.log" 2>&1 || { echo "❌ pxpkg init 失败（rc=$?）"; tail -10 "$W/init.log"; }
echo "  --- init：px.toml 前 5 行 ---"; head -5 px.toml 2>&1
okc=0; errlist=""; addfail=""
for i in $(seq 0 $((N-1))); do
    p="${PKGS[$i]}"
    if ! "$ROOT/tools/pxpkg" add "$p@0.1.0" > "$W/add_$p.log" 2>&1; then addfail="$addfail $p"; fi
done
[ -n "$addfail" ] && echo "❌ pxpkg add 失败 $(( $(echo $addfail | wc -w) )) 包：$addfail"
[ -n "$addfail" ] && { echo "  --- 首个失败包的 add 输出 ---"; tail -15 "$W/add_$(echo $addfail | awk '{print $1}').log"; }
# M193-DIAG：CI（run for 1a8b28c）上本步 `pxpkg install` 中断于第 22 个包（functools），
#   而本地全量门同一步**绿** ⇒ 环境相关（CI runner 资源/时序）。当时注解只拿到
#   stdout 尾巴，看不到 rc 与 stderr ⇒ 这里把 **rc / stderr / 装了几个** 全部无条件打出
#   （job 日志非管理员 403 不可读，注解是唯一通道 ⇒ 诊断必须"少而准"且**每次**都在）。
irc=0
"$ROOT/tools/pxpkg" install > "$W/install.log" 2>"$W/install.err" || irc=$?
{
if [ "$irc" != 0 ]; then
    echo "❌ pxpkg install 失败（rc=$irc）"
    echo "  --- install stderr 尾部 15 行 ---"; tail -15 "$W/install.err"
fi
echo "  --- install 输出尾部 20 行 ---"; tail -20 "$W/install.log"
echo "  --- install 进度：已打印「安装」行 $(grep -c '安装 ' "$W/install.log" 2>/dev/null) / 期望 $N；rc=$irc ---"
[ -s "$W/install.err" ] && { echo "  --- install stderr 非空（尾 5 行） ---"; tail -5 "$W/install.err"; }
echo "  --- .px_modules 实测 ---"; ls .px_modules 2>&1 | head -8; echo "  (目录数=$(ls .px_modules 2>/dev/null | wc -l))"
echo "  --- px.toml 依赖段 ---"; grep -c '=' px.toml 2>/dev/null
} 2>&1 | tee "$W/diag.txt"
# M195：上面这段诊断此前只进 **stdout**（= CI 的 job 日志，非管理员 403 读不到）⇒ 红的时候
#   拿到注解也看不见 rc/stderr。现在**同时落 $W/diag.txt**，ci.yml 的注解步会把它收进注解。
n_inst="$(ls .px_modules 2>/dev/null | wc -l)"
chk "[2] pxpkg 一次装齐 $N 包（.px_modules 目录数 = $N）" "[ \"$n_inst\" = \"$N\" ]"
n_entry=0
for i in $(seq 0 $((N-1))); do
    p="${PKGS[$i]}"
    [ -f ".px_modules/$p/$p.px" ] && n_entry=$((n_entry+1))
done
chk "[2] 每包入口 .px_modules/<pkg>/<pkg>.px 都到位（$n_entry/$N）" "[ $n_entry = $N ]"
n_files=0; n_reg=0
for i in $(seq 0 $((N-1))); do
    p="${PKGS[$i]}"; v="${VERS[$i]}"
    n_files=$((n_files + $(ls ".px_modules/$p"/*.px 2>/dev/null | wc -l)))
    n_reg=$((n_reg + $(ls "$REG/$p/$v"/*.px 2>/dev/null | wc -l)))
done
chk "[2] 安装的 .px 文件总数 = registry 侧总数（$n_files = $n_reg；多文件包不丢件）" "[ $n_files = $n_reg ]"
for i in $(seq 0 $((N-1))); do
    p="${PKGS[$i]}"
    printf 'import %s\n\ndef main():\n    print("ok")\n' "$p" > main.px
    if timeout 60 "$ROOT/bootstrap/pxi" run main.px > "$W/imp_$p.out" 2>&1 && grep -qx 'ok' "$W/imp_$p.out"; then
        okc=$((okc+1))
    else
        errlist="$errlist $p"
    fi
done
chk "[2] $N 包全部 import 成功（解释轨）$errlist" "[ $okc = $N ]"

echo "=== [3] 抽样：双轨**编译**跑通（含全部多文件包 + 二进制/解析族）"
SUBSET=(qrcode xlsx mysql base58 checksum tar glob csv jwt uuid decimal toml template)
[ "$LIMIT" != 0 ] && SUBSET=("${SUBSET[@]:0:$LIMIT}")
# 前置：抽样名**必须**都是本轮引入的包（否则测的是 stdlib 老包 ⇒ 假判据；
#   实测踩过：初版把 `yaml` 写进抽样，而 yaml 是 registry 里**原有**的 stdlib 镜像、不在引入清单）
sub_bad=""
for p in "${SUBSET[@]}"; do grep -qE "^\| *$p *\|" "$PROV" || sub_bad="$sub_bad $p"; done
chk "[3] 抽样名都在引入清单里（不在 ⇒ 前置失败）$sub_bad" "[ -z \"$sub_bad\" ]"
ok3=0; bad3=""
for p in "${SUBSET[@]}"; do
    printf 'import %s\n\ndef main():\n    print("ok")\n' "$p" > main.px
    # ⚠️ 判据不能只看 rc：**编译轨缺模块是「警告 + 跳过」，rc 仍为 0、运行也照样打印 ok**
    #    （实测 `[警告] 找不到模块 'qr_core.px'（已跳过 → 运行期将报未定义）`）。
    #    ⇒ 必须同时断言「编译日志无『找不到模块』」+「安装件文件数与 registry 一致」。
    ai="$(ls ".px_modules/$p"/*.px 2>/dev/null | wc -l)"; ar="$(ls "$REG/$p/0.1.0"/*.px 2>/dev/null | wc -l)"
    rm -rf build; timeout 300 "$ROOT/tools/px" build main.px > "$W/b_$p.log" 2>&1
    if [ -x build/main ] && [ "$(./build/main)" = "ok" ] && ! grep -q '找不到模块' "$W/b_$p.log" && [ "$ai" = "$ar" ]; then
        rm -rf build
        PX_BUILD_ENGINE=c timeout 300 "$ROOT/tools/px" build main.px > "$W/bc_$p.log" 2>&1
        if [ -x build/main ] && [ "$(./build/main)" = "ok" ] && ! grep -q '找不到模块' "$W/bc_$p.log"; then ok3=$((ok3+1)); else bad3="$bad3 $p(C)"; fi
    else bad3="$bad3 $p(VM)"; fi
    rm -rf build
done
chk "[3] 抽样 ${#SUBSET[@]} 包双轨编译+运行全通过 $bad3" "[ $ok3 = ${#SUBSET[@]} ]"

echo "=== [4] 多文件包语义：安装/导入/--locked 篡改检测"
MF="$W/mf"; rm -rf "$MF"; mkdir -p "$MF/reg/multi/0.1.0" "$MF/app"
cat > "$MF/reg/multi/0.1.0/multi_util.px" <<'EOF'
def mu_double(x):
    return x * 2
EOF
cat > "$MF/reg/multi/0.1.0/multi.px" <<'EOF'
import "multi_util.px"

def multi_quad(x):
    return mu_double(mu_double(x))
EOF
cd "$MF/app"
PX_REGISTRY="$MF/reg" "$ROOT/tools/pxpkg" init --name mf > /dev/null 2>&1
PX_REGISTRY="$MF/reg" "$ROOT/tools/pxpkg" add multi@^0.1.0 > /dev/null 2>&1
PX_REGISTRY="$MF/reg" "$ROOT/tools/pxpkg" install > "$W/mf_install.log" 2>&1
chk "[4] 两文件包安装出 2 个 .px" "[ \"\$(ls .px_modules/multi/*.px | wc -l)\" = 2 ]"
printf 'import multi\n\ndef main():\n    print("quad(3) = " + str(multi_quad(3)))\n' > main.px
r_i="$(timeout 60 "$ROOT/bootstrap/pxi" run main.px 2>&1)"
rm -rf build; timeout 300 "$ROOT/tools/px" build main.px > /dev/null 2>&1; r_vm="$(./build/main 2>&1)"; rm -rf build
PX_BUILD_ENGINE=c timeout 300 "$ROOT/tools/px" build main.px > /dev/null 2>&1; r_c="$(./build/main 2>&1)"; rm -rf build
chk "[4] 三轨一致 quad(3)=12（I「$r_i」）" "[ \"$r_i\" = 'quad(3) = 12' ] && [ \"$r_vm\" = \"$r_i\" ] && [ \"$r_c\" = \"$r_i\" ]"
PX_REGISTRY="$MF/reg" "$ROOT/tools/pxpkg" install --locked > /dev/null 2>&1 && mf_ok=1 || mf_ok=0
chk "[4] --locked 对未篡改包通过" "[ $mf_ok = 1 ]"
printf 'def mu_double(x):\n    return 999\n' > .px_modules/multi/multi_util.px
PX_REGISTRY="$MF/reg" "$ROOT/tools/pxpkg" install --locked > "$W/mf_locked.log" 2>&1 && tam=0 || tam=1
chk "[4] **辅助文件**被篡改 ⇒ --locked 拒绝（修前只查入口）" "[ $tam = 1 ] && grep -q '篡改' \"$W/mf_locked.log\""

echo
if [ "$NEG" = 1 ]; then
    echo "=== [5] 负控（各自独立判红；被改文件逐字节还原）"
    VICTIM="$REG/${PKGS[0]}/${VERS[0]}/${PKGS[0]}.px"
    cp -f "$VICTIM" "$BACK/entry.px"; cp -f "$PROV" "$BACK/THIRD_PARTY.md"
    AUX="$(ls $REG/qrcode/0.1.0/*.px | grep -v '/qrcode.px$' | head -1)"
    cp -f "$AUX" "$BACK/aux.px"
    restore_all() { cp -f "$BACK/entry.px" "$VICTIM"; cp -f "$BACK/THIRD_PARTY.md" "$PROV"; cp -f "$BACK/aux.px" "$AUX"; }
    trap 'restore_all' EXIT
    sha_v() { sha256sum "$VICTIM" | cut -c1-16; }
    sha_p() { sha256sum "$PROV" | cut -c1-16; }
    sha_a() { sha256sum "$AUX" | cut -c1-16; }
    V0="$(sha_v)"; P0="$(sha_p)"; A0="$(sha_a)"

    layer1_ok() {   # 返回 0 = 第 ① 层为绿（负控生效时应为 1）；空转（解析不到行）也算判红
        local cnt=0 okr=0
        while IFS='|' read -r _ pkg ver nf sha _rest; do
            pkg="$(echo "$pkg" | tr -d ' ')"; ver="$(echo "$ver" | tr -d ' ')"
            nf="$(echo "$nf" | tr -d ' ')"; sha="$(echo "$sha" | tr -d ' `')"
            case "$pkg" in ''|'包'|'---'*) continue ;; esac
            cnt=$((cnt+1))
            d="$REG/$pkg/$ver"
            [ -d "$d" ] || { okr=1; continue; }
            a_n="$(ls "$d"/*.px 2>/dev/null | wc -l)"; [ "$a_n" = "$nf" ] || okr=1
            a_s="$(sha256sum "$d/$pkg.px" 2>/dev/null | cut -c1-16)"; [ "$a_s" = "$sha" ] || okr=1
        done < <(grep '^|' "$PROV")
        [ "$cnt" = "$N" ] || okr=1
        return $okr
    }
    smoke_ok() {   # 返回 0 = 抽样的双轨全通过（负控时 C 应失败 ⇒ 返回 1）
        # ⚠️ 必须**先重装**（pxpkg install 会按 registry 现状重铺 .px_modules）——
        #    否则判据读的是第 [2] 层装好的旧副本，registry 侧被删文件影响不到它 ⇒ 假绿。
        local p=0
        cd "$APP"
        for lib in qrcode xlsx; do
            PX_REGISTRY="$REG" "$ROOT/tools/pxpkg" add "$lib@0.1.0" > /dev/null 2>&1
        done
        PX_REGISTRY="$REG" "$ROOT/tools/pxpkg" install > /dev/null 2>&1
        for lib in qrcode xlsx; do
            cd "$APP"; printf 'import %s\n\ndef main():\n    print("ok")\n' "$lib" > main.px
            rm -rf build; timeout 300 "$ROOT/tools/px" build main.px > "$W/nc_$lib.log" 2>&1
            if [ -x build/main ] && [ "$(./build/main)" = "ok" ] && ! grep -q '找不到模块' "$W/nc_$lib.log"; then p=$((p+1)); fi
            rm -rf build
        done
        [ "$p" = 2 ]
    }
    neg() {   # neg <名> <判据函数(0=负控生效)> <打桩函数> <被改文件>
        local name="$1" fn="$2" patcher="$3" file="$4" before="$5"
        restore_all
        $patcher || { echo "  FAIL 负控 $name：打桩失败"; fail=$((fail+1)); restore_all; return; }
        local cur
        cur="$( [ -f "$file" ] && sha256sum "$file" | cut -c1-16 || echo MISSING )"
        if [ "$cur" = "$before" ]; then
            echo "  FAIL 负控 $name：打桩未生效（锚点/文件未变）"; fail=$((fail+1)); restore_all; return
        fi
        # ⚠️ 判据串必须走 `eval`：`! layer1_ok` 里的 `!` 若来自变量展开，
        #    bash **不认**它是保留字（会当命令名 ⇒ `!: command not found`）⇒ 假绿。
        #    （同族教训：M185 的 `restore_all` 误调、M48 的「覆盖变量式负控」。）
        if eval "$fn"; then
            echo "  PASS 负控 $name：判据已判红（符合预期）"; pass=$((pass+1))
        else
            echo "  FAIL 负控 $name：判据仍为绿（负控没有牙）"; fail=$((fail+1))
        fi
        restore_all
    }
    patch_entry() { printf '\n# M187-NEG-A\n' >> "$VICTIM"; }
    patch_prov()  { sed -i "s/\`$(sha256sum "$VICTIM" | cut -c1-16)\`/\`0000000000000000\`/" "$PROV"; }
    patch_aux()   { rm -f "$AUX"; }

    neg A "! layer1_ok" patch_entry "$VICTIM" "$V0"
    neg B "! layer1_ok" patch_prov  "$PROV"   "$P0"
    neg C "! smoke_ok"  patch_aux   "$AUX"    "$A0"
    restore_all
    [ "$(sha_v)" = "$V0" ] && [ "$(sha_p)" = "$P0" ] && [ "$(sha_a)" = "$A0" ] || { echo "  FAIL 源未逐字节还原"; fail=$((fail+1)); }
else
    echo "=== [5] 负控已跳过（--neg-skip）"
fi

rm -rf "$HERE/build"
echo
echo "M187 门结果：PASS=$pass FAIL=$fail"
[ "$fail" = 0 ] && echo "M187-VERIFY-OK" || echo "M187-VERIFY-FAILED"
exit $([ "$fail" = 0 ] && echo 0 || echo 1)
