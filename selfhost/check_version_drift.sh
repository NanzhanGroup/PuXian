#!/usr/bin/env bash
# ============================================================
# selfhost/check_version_drift.sh —— 版本链守卫（M368 · 缺陷 638）
# ------------------------------------------------------------
# 为什么要有它：
#   `packaging/bump_version.sh` 能改 9 个文件的版本串，但**没有任何一步强制调它**
#   ⇒ `tools/px SELFHOST_VER` 从 0.2.281 一路冻到 v0.2.367（84 个里程碑没涨）：
#   源码直跑 `./tools/px --version` 报 0.2.281、发布包是 0.2.367 —— 对外两个数，
#   而所有门全绿（它们量行为，不量「版本号还活着没有」）。
#
# 判据（任何一条不满足即红）：
#   E0 各处锚点必须**取得到**（写法变了就红 —— 否则下面的比较会变成空对比 = 恒绿）
#   E1 版本串取值**处处相同**（6 文件 7 个锚点）
#   E2 与 git 最新 tag 的里程碑差 ≤ 1（允许「本轮正在建的那一版」）
#      读不到 tag（CI 浅克隆 / 非 git 树）⇒ **SKIP 并打印**，不当绿
#
# 自成自证（--self-test）：5 条用例，含 3 条负控（证明判据有鉴别力）
#
# 用法：bash selfhost/check_version_drift.sh             # 判本仓
#       bash selfhost/check_version_drift.sh --self-test # 判具自证
# 退出码：0=绿/自证通过 · 1=红 · 2=用法/环境错
# ============================================================
set -uo pipefail

# ── 锚点表：文件|ERE（第 1 组是版本号）──
ANCHORS=(
  'tools/px|^SELFHOST_VER="([0-9]+\.[0-9]+\.[0-9]+)"'
  'selfhost/compiler.px|let PXC_VER = "([0-9]+\.[0-9]+\.[0-9]+)"'
  'selfhost/interp.px|let PXI_VER = "([0-9]+\.[0-9]+\.[0-9]+)"'
  'tools/pxlsp.px|let PXLSP_VER = "([0-9]+\.[0-9]+\.[0-9]+)"'
  'tools/pxmcp.px|let PXMC_VER = "([0-9]+\.[0-9]+\.[0-9]+)"'
  'tools/pxmcp.px|let PXC_VER_INFO = "pxc ([0-9]+\.[0-9]+\.[0-9]+)'
  'tools/pxfmt.px|print\("pxfmt ([0-9]+\.[0-9]+\.[0-9]+)'
)

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }

# 跑一次判定：$1=根目录 $2=tag 源（空=读 git）  → 打印并给出结论串
check_root() {  # 退出码 0=绿 1=红
    local root="$1" tagsrc="${2:-}" f re val base="" nval=0 f_ok=0
    local -a names=() vals=()
    echo "── 判定根：$root"
    for spec in "${ANCHORS[@]}"; do
        f="${spec%%|*}"; re="${spec#*|}"
        if [ ! -f "$root/$f" ]; then bad "缺文件 $f"; nval=$((nval+1)); continue; fi
        val="$(grep -oE "$re" "$root/$f" 2>/dev/null | head -1 | sed -E "s/$re/\1/")"
        if [ -z "$val" ]; then bad "$f：锚点取不到（判据写法失效，先修判据）"; nval=$((nval+1)); continue; fi
        names+=("$f"); vals+=("$val"); f_ok=$((f_ok+1))
    done
    if [ "$f_ok" -gt 0 ]; then base="${vals[0]}"; fi
    local i=0
    while [ "$i" -lt "${#vals[@]}" ]; do
        if [ "${vals[$i]}" = "$base" ]; then ok "${names[$i]} = ${vals[$i]}"
        else bad "${names[$i]} = ${vals[$i]}（基准 $base）"; fi
        i=$((i+1))
    done
    # E2
    SKIP_TAGS=0
    if [ -z "$base" ]; then
        bad "无可用版本串 ⇒ E2 判不了 ⇒ 整体红"
    else
        local t="$tagsrc"
        if [ -z "$t" ]; then t="$(git -C "$root" tag --list 'v[0-9]*.[0-9]*.[0-9]*' --sort=-v:refname 2>/dev/null | head -3 | tr '\n' ' ')"; fi
        if [ -z "${t// /}" ]; then
            SKIP_TAGS=1
            note "SKIP：读不到任何 vX.Y.Z tag（CI 浅克隆/非 git 树）—— E2 **没有验到**，不算绿"
        else
            local tg p d
            tg="$(printf '%s\n' $t | head -1)"
            p="${base##*.}"
            local tp; tp="$(printf '%s' "$tg" | sed -nE 's/^v[0-9]+\.[0-9]+\.([0-9]+)$/\1/p')"
            if [ -z "$tp" ]; then bad "tag '$tg' 解析不出版本号"; else
                d=$(( p - tp )); [ "$d" -lt 0 ] && d=$(( -d ))
                if [ "$d" -le 1 ]; then ok "源码 $base ⇄ 最新 tag $tg（差 $d ≤ 1）"
                else
                    # M386（缺陷 694）：允许「合并发版」—— 判据贴**契约**而不是贴数字：
                    #   真正的契约是「每一个未发版的里程碑都必须登记」，不是「差必须 ≤1」。
                    #   修前形态：M385 只有 CHANGELOG 改动（重烘 70 分钟不划算）⇒ 并入 v0.2.386，
                    #   源码 0.2.386 ⇄ tag v0.2.384 差 2 ⇒ 门红 ⇒ **发布轨被自己的门锁死**
                    #   （门绿才能发，而发之前 tag 必然落后）。这是判据与流程互锁，不是漂移。
                    #   登记制：selfhost/golden/version_skips.txt 每行「<里程碑号> <原因>」。
                    #   负控：没登记而差 2 ⇒ 照样红（N5）；登记数不够 ⇒ 红。
                    local skipped=0 mid
                    if [ -f "$root/selfhost/golden/version_skips.txt" ]; then
                        mid=$(( tp + 1 ))
                        while [ "$mid" -lt "$p" ]; do
                            if grep -qE "^[[:space:]]*${mid}([[:space:]]|$)" "$root/selfhost/golden/version_skips.txt" 2>/dev/null; then
                                skipped=$(( skipped + 1 ))
                            fi
                            mid=$(( mid + 1 ))
                        done
                    fi
                    if [ "$(( d - 1 ))" -le "$skipped" ]; then
                        ok "源码 $base ⇄ 最新 tag $tg（差 $d；其中 $skipped 个里程碑已登记「并入后版」）"
                    else
                        bad "源码 $base ⇄ 最新 tag $tg（差 $d > 1 + 已登记 $skipped）⇒ 版本链已漂，跑 packaging/bump_version.sh 或登记 selfhost/golden/version_skips.txt"
                    fi
                fi
            fi
        fi
    fi
    if [ "$FAIL" -eq 0 ] && [ "$nval" -eq 0 ]; then
        if [ "$SKIP_TAGS" = 1 ]; then echo "VERSION-DRIFT=SKIP(no-tags)"; else echo "VERSION-DRIFT=OK"; fi
        return 0
    fi
    echo "VERSION-DRIFT=RED"; return 1
}

# ── 自证：造夹具 —— 正控 1 + 负控 3 + SKIP 1（计数独立，不受 check_root 的全局计数污染）──
selftest() {
    local T SOK=0 SBAD=0
    T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
    mkfix() {  # $1=目录 $2=统一版本
        local d="$1" v="$2"
        mkdir -p "$d/tools" "$d/selfhost"
        printf 'SELFHOST_VER="%s"\n' "$v" > "$d/tools/px"
        printf 'let PXC_VER = "%s"\n' "$v" > "$d/selfhost/compiler.px"
        printf 'let PXI_VER = "%s"\n' "$v" > "$d/selfhost/interp.px"
        printf 'let PXLSP_VER = "%s"\n' "$v" > "$d/tools/pxlsp.px"
        printf 'let PXMC_VER = "%s"\nlet PXC_VER_INFO = "pxc %s (普贤 PuXian · selfhosted M-B9a)"\n' "$v" "$v" > "$d/tools/pxmcp.px"
        printf 'print("pxfmt %s (普贤 PuXian fmt · M64a 自举)")\n' "$v" > "$d/tools/pxfmt.px"
    }
    s_ok()  { SOK=$((SOK+1));  printf '  ✅ %s\n' "$*"; }
    s_bad() { SBAD=$((SBAD+1)); printf '  ❌ %s\n' "$*"; }
    # 跑判定并把它的 ok/bad 输出藏掉（只看退出码与结论串）；同时把全局计数清零
    run_q() { local out rc=0; out="$( check_root "$1" "${2:-}" 2>&1 )" || rc=$?; printf '%s' "$out"; return $rc; }

    echo "【T1】正控：六处一致 0.2.368 · tag v0.2.367 ⇒ 必须绿"
    mkfix "$T/t1" 0.2.368
    PASS=0; FAIL=0; out="$( run_q "$T/t1" v0.2.367 )" || true
    if grep -q 'VERSION-DRIFT=OK' <<< "$out"; then s_ok "T1 绿（负控基线）"
    else s_bad "T1 应绿实非绿：$(printf '%s' "$out" | tail -1)"; fi

    echo "【N1】负控：selfhost/interp.px 单独改成 0.2.367 ⇒ 必须红"
    mkfix "$T/n1" 0.2.368
    printf 'let PXI_VER = "0.2.367"\n' > "$T/n1/selfhost/interp.px"
    PASS=0; FAIL=0; out="$( run_q "$T/n1" v0.2.367 )" || true
    if grep -q 'VERSION-DRIFT=RED' <<< "$out"; then s_ok "N1 红（E1 抓到单点漂移）"
    else s_bad "N1 应红实绿 ⇒ E1 无鉴别力"; fi

    echo "【N2】负控：tools/pxfmt.px 锚点写法被改（取不到）⇒ 必须红，且不许判 OK"
    mkfix "$T/n2" 0.2.368
    printf 'print("pxfmt v0.2.368")\n' > "$T/n2/tools/pxfmt.px"
    PASS=0; FAIL=0; out="$( run_q "$T/n2" v0.2.367 )" || true
    if grep -q 'VERSION-DRIFT=RED' <<< "$out" && grep -q '锚点取不到' <<< "$out"; then
        s_ok "N2 红（E0 锚点失效即红，不静默空对比）"
    else s_bad "N2 应红实绿 ⇒ E0 无鉴别力"; fi

    echo "【N3】负控：版本停在 0.2.364 而最新 tag v0.2.367（差 3）⇒ 必须红"
    mkfix "$T/n3" 0.2.364
    PASS=0; FAIL=0; out="$( run_q "$T/n3" v0.2.367 )" || true
    if grep -q 'VERSION-DRIFT=RED' <<< "$out"; then s_ok "N3 红（E2 抓到版本链停摆）"
    else s_bad "N3 应红实绿 ⇒ E2 无鉴别力"; fi

    echo "【N4】负控：读不到 tag ⇒ 必须 SKIP（既不是 OK 也不许红）"
    mkfix "$T/n4" 0.2.368
    PASS=0; FAIL=0; out="$( run_q "$T/n4" " " )" || true
    if grep -q 'VERSION-DRIFT=SKIP(no-tags)' <<< "$out" \
       && ! grep -q 'VERSION-DRIFT=OK' <<< "$out"; then
        s_ok "N4 SKIP（明示「没有验到」）"
    else s_bad "N4 期望 SKIP，实得 $(printf '%s' "$out" | tail -1)"; fi

    echo "【T2】正控：源码 0.2.369 · tag v0.2.367（差 2）但 368 已登记并入 ⇒ 必须绿"
    mkfix "$T/t2" 0.2.369
    mkdir -p "$T/t2/selfhost/golden"
    printf '368 M368 未单独发版，并入后版（自测夹具）\n' > "$T/t2/selfhost/golden/version_skips.txt"
    PASS=0; FAIL=0; out="$( run_q "$T/t2" v0.2.367 )" || true
    if grep -q 'VERSION-DRIFT=OK' <<< "$out"; then s_ok "T2 绿（登记制放行合并发版）"
    else s_bad "T2 应绿实红：$(printf '%s' "$out" | tail -1)"; fi

    echo "【N5】负控：源码 0.2.369 · tag v0.2.367（差 2）且 368 **未登记** ⇒ 必须红"
    mkfix "$T/n5" 0.2.369
    PASS=0; FAIL=0; out="$( run_q "$T/n5" v0.2.367 )" || true
    if grep -q 'VERSION-DRIFT=RED' <<< "$out"; then s_ok "N5 红（未登记不得放行 —— 登记制有牙）"
    else s_bad "N5 应红实绿 ⇒ 登记制无鉴别力"; fi

    echo
    if [ "$SBAD" -eq 0 ] && [ "$SOK" -eq 7 ]; then
        echo "VERSION-DRIFT-SELFTEST=OK（$SOK 条：2 正控 + 4 负控 + 1 SKIP）"; exit 0
    fi
    echo "VERSION-DRIFT-SELFTEST=RED（ok=$SOK bad=$SBAD）"; exit 1
}

case "${1:-}" in
    --self-test) selftest ;;
    -h|--help) sed -n '2,26p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    '') ROOT="$(cd "$(dirname "$0")/.." && pwd)"; check_root "$ROOT" "${VD_TAGS:-}" ;;
    *)  check_root "$1" "${VD_TAGS:-}" ;;
esac
