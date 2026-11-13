#!/usr/bin/env bash
# ============================================================
# selfhost/check_orphan_gates.sh —— **孤儿门守卫**（M249 建立 · 缺陷 428 · M401 加固 缺陷 766）
# ------------------------------------------------------------
# 判据：`examples/*/verify.sh` 里每一扇门必须被**至少一个运行器**收录
#       （`selfhost/gates.registry.sh` 或 `.github/workflows/ci.yml`），
#       否则必须出现在 `selfhost/orphan_gates.txt`（**带理由**）；基线**只许缩小**。
#
# M401（缺陷 766）：**「提起路径」曾被当成「收录」** —— 提取器原先是
#   `grep -oE 'examples/<名>/verify\.sh'`（全文件任意位置），于是一句注释
#   「覆盖它的门 examples/xxx/verify.sh 不在任何运行器里」竟然把 xxx 记成**已收录**
#   ⇒ 孤儿集变小 ⇒ 判据假绿（本轮实锤：我在 registry 里写这句注释，m90_s1 当场
#   从孤儿变成「已收录」，守卫反而报「基线已过期」）。
#   加固口径（与 check_gate_registry.sh 同源，两门认同一件事）：
#     registry 只认**行首 `run <名> bash examples/…/verify.sh`**；
#     ci.yml 只认**非注释行**里的 `bash examples/…/verify.sh`。
#   负控（--self-test）：把路径写进 registry 注释 / ci.yml 注释行 ⇒ 仍须判孤儿。
#
# 为什么要有它（M249 实测数据）：
#   `check_gate_registry.sh`（M243）比的是 **registry ⇄ ci.yml**，
#   而「**两边都没提到**」的门**不在任何差集里** ⇒ 那半条判据**看不见**。
#   实测：179 个 verify.sh ⇄ registry 124 / ci.yml 127
#        ⇒ **55 扇门没有任何运行器跑它们**（其中 4 扇 ci.yml 有 ⇒ 真孤儿 51）。
#
# 后果（= 用户 2026-10-03 点名修的那条，就是这个形状）：
#   `m63_langfix` 的判据写着 `grep "pxc 0.1.0"`，实测早已是 `0.2.0`
#   ⇒ **判据长期恒红**；它依赖的 `/tmp/m63_mock.py` 被清理后也**无人发现**
#   ⇒ 门实际早已跑不起来。**孤儿门 = 判据腐烂的温床**（没人跑 ⇒ 没人发现 ⇒ 没人修）。
#
# 用法：./selfhost/check_orphan_gates.sh [--self-test]
# 退出码：0 绿 · 1 红 · 2 参数/环境错
# ============================================================
set -uo pipefail

SELF_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="${ORPHAN_ROOT:-$(cd "$SELF_DIR/.." && pwd)}"
# 夹具模式（--self-test 用）：跳过规模锚点（小夹具天然达不到 150/100/40）
FIXTURE="${ORPHAN_SELFTEST:-0}"
cd "$ROOT" || { echo "❌ 进不去仓库根 $ROOT" >&2; exit 2; }

EX_DIR="${ORPHAN_EX_DIR:-examples}"
REG="${ORPHAN_REGISTRY:-selfhost/gates.registry.sh}"
CI="${ORPHAN_CI:-.github/workflows/ci.yml}"
BASE="${ORPHAN_BASELINE:-selfhost/orphan_gates.txt}"
# 基线规模上限 —— 写死常量（**只许缩小**：要扩充就得改这个数字，留下痕迹）
ORPHAN_MAX="${ORPHAN_MAX:-15}"

SELFTEST=0
for a in "$@"; do
    case "$a" in
        --self-test) SELFTEST=1 ;;
        -h|--help)   sed -n '2,40p' "$0"; exit 0 ;;
        *)           echo "未知参数：$a（可用：--self-test）" >&2; exit 2 ;;
    esac
done

# ── M401（缺陷 766）自证：注释里的路径**不得**被当成收录 ────────────────────
#   夹具 3 门：g_really（真 `run` 行收录）· g_comment（**只在注释里**被提及 ⇒ 必须仍是孤儿）
#               · g_net（基线过期负控用）
do_selftest() {
    local base="$SELF_DIR/check_orphan_gates.sh"
    local F rc sp=0 sf=0 out
    chk() { if [ "$2" = "$3" ]; then echo "  ✅ $1"; sp=$((sp+1)); else echo "  ❌ $1（期望 rc=$3 实得 rc=$2）"; sf=$((sf+1)); fi; }
    F="$(mktemp -d)"; trap 'rm -rf "$F"' RETURN
    mkfix() {   # $1=目录；$2=基线内容；$3=registry 内容；$4=台账内容（缺省=一条 OK）
        mkdir -p "$F/$1/examples/g_really" "$F/$1/examples/g_comment" "$F/$1/selfhost" "$F/$1/.github/workflows"
        : > "$F/$1/examples/g_really/verify.sh"
        : > "$F/$1/examples/g_comment/verify.sh"
        printf '%s\n' "$3" > "$F/$1/selfhost/gates.registry.sh"
        printf '%s\n' '      # 注释行：bash examples/g_comment/verify.sh（不算收录）' \
                      '      - run: bash examples/g_really/verify.sh' \
                      > "$F/$1/.github/workflows/ci.yml"
        printf '%s\n' "$2" > "$F/$1/selfhost/orphan_gates.txt"
        if [ "$#" -ge 4 ]; then
            printf '%s' "$4" > "$F/$1/selfhost/orphan_audit.tsv"
        else
            printf 'g_comment\t纯自包含\t0\t0\tOK\t实测通过（夹具）\n' > "$F/$1/selfhost/orphan_audit.tsv"
        fi
    }
    runfix() { ORPHAN_ROOT="$F/$1" ORPHAN_SELFTEST=1 bash "$base" 2>&1; }
    local REG_OK='run g_really bash examples/g_really/verify.sh
# 覆盖它的门 examples/g_comment/verify.sh 早就不在任何运行器里（注释不算收录）'
    local BASE_OK='g_comment|夹具理由：只被注释提到，必须仍然算孤儿（缺陷 766 负控）'

    mkfix ok "$BASE_OK" "$REG_OK"
    out="$(runfix ok)"; rc=$?
    chk "T1 注释提及不算收录（g_comment 仍计孤儿 · 基线成立 ⇒ 绿）" "$rc" 0
    grep -q '孤儿门（两个运行器都没收录）= 1' <<< "$out" \
        && { echo "  ✅ T1b 孤儿计数 = 1（注释里的 g_comment 没被当成已收录）"; sp=$((sp+1)); } \
        || { echo "  ❌ T1b 孤儿计数不是 1 ⇒ 提取器又把注释当收录了"; sf=$((sf+1)); }

    mkfix nb "" "$REG_OK"
    out="$(runfix nb)"; rc=$?
    chk "N1 负控：g_comment 不在基线 ⇒ 必判红（新增孤儿）" "$rc" 1
    grep -q '新增孤儿 1 个' <<< "$out" && { echo "  ✅ N1b 红得有名字（新增孤儿 1 个）"; sp=$((sp+1)); } \
        || { echo "  ❌ N1b 判红了但没说清是哪条"; sf=$((sf+1)); }

    mkfix st "$BASE_OK
g_ghost|夹具理由：这一条已被收录或不存在 ⇒ 过期必须判红" "$REG_OK"
    out="$(runfix st)"; rc=$?
    chk "N2 负控：基线里的 g_ghost 不成立 ⇒ 必判红（过期）" "$rc" 1

    mkfix cov "" "$REG_OK
run g_comment bash examples/g_comment/verify.sh" ""
    out="$(runfix cov)"; rc=$?
    chk "N3 正控：真「run」行收录 ⇒ 该门不再算孤儿（基线为空仍绿）" "$rc" 0

    # N4/N4b（M403）：批量复活清单只有在**消费者已注册且脚本在盘**时才算收录 ——
    #   否则清单就成了「把孤儿洗白」的新后门（M401 缺陷 766 是注释洗白，这是清单洗白）。
    preprint() {   # 在 $1 夹具里加：一扇 g_revive + 一份清单
        mkdir -p "$F/$1/examples/g_revive" "$F/$1/examples/m403_orphan_batch"
        : > "$F/$1/examples/g_revive/verify.sh"
        : > "$F/$1/examples/m403_orphan_batch/verify.sh"
        printf 'g_revive\n' > "$F/$1/selfhost/orphan_revived.txt"
    }
    mkfix rv "$BASE_OK" "$REG_OK
run m403_orphan_batch bash examples/m403_orphan_batch/verify.sh"
    preprint rv
    out="$(runfix rv)"; rc=$?
    chk "N4 正控：清单 + 消费者已注册且脚本在盘 ⇒ 清单内门算已收录（g_comment 仍在基线 ⇒ 绿）" "$rc" 0

    mkfix rv2 "$BASE_OK" "$REG_OK"
    preprint rv2
    out="$(runfix rv2)"; rc=$?
    chk "N4b 负控：清单在但消费者未注册 ⇒ 不算收录（新增孤儿必判红）" "$rc" 1
    #   点名判据用「输出里出现 g_revive」而不是计数：夹具里 m403_orphan_batch 自身也是一个
    #   example 目录（未注册时同样算孤儿）⇒ 计数会随夹具形状变，名字不会。
    grep -q 'g_revive' <<< "$out" \
        && { echo "  ✅ N4c 负控红得有名字（g_revive 被点为新增孤儿）"; sp=$((sp+1)); } \
        || { echo "  ❌ N4c 判红了但没点名"; sf=$((sf+1)); }

    echo "── check_orphan_gates --self-test：通过 $sp · 失败 $sf"
    [ "$sf" = 0 ]
}
if [ "$SELFTEST" = 1 ]; then do_selftest; exit $?; fi

PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT

echo "=== 孤儿门守卫（M249 · 缺陷 428）==="

# ── 1 三向提取（全从源码派生，不手抄任何名单）──
#   M401（缺陷 766）：**只认真正的登记点**
#     · registry：行首 `run <名> bash examples/…/verify.sh`（注释行一律不算）
#     · ci.yml  ：非注释行里的 `bash examples/…/verify.sh`
#   与 check_gate_registry.sh 的口径同源 —— 同一条「什么算收录」，两门认同一件事。
find "$EX_DIR" -mindepth 2 -maxdepth 2 -name verify.sh 2>/dev/null \
    | sed 's|^\./||' | awk -F/ '{print $2}' | sort -u > "$T/all.txt"
grep -E '^[[:space:]]*run[[:space:]]+[^[:space:]]+[[:space:]]+.*examples/[A-Za-z0-9_-]+/verify\.sh' "$REG" 2>/dev/null \
    | grep -oE 'examples/[A-Za-z0-9_-]+/verify\.sh' \
    | sed 's|examples/||; s|/verify\.sh||' | sort -u > "$T/reg.txt"
grep -vE '^[[:space:]]*#' "$CI" 2>/dev/null \
    | grep -oE '(bash|run)[[:space:]]+[^[:space:]]*examples/[A-Za-z0-9_-]+/verify\.sh' \
    | grep -oE 'examples/[A-Za-z0-9_-]+/verify\.sh' \
    | sed 's|examples/||; s|/verify\.sh||' | sort -u > "$T/ci.txt"
cat "$T/reg.txt" "$T/ci.txt" | sort -u > "$T/covered.txt"

# ── 1b 批量复活清单（M403）：清单里的门由 examples/m403_orphan_batch **串行实跑** ──
#   与「注释里写一句路径就算收录」的区别（M401 缺陷 766 的教训）：清单要被算作**收录**，
#   必须先证明**消费者真的注册并存在**（registry 有 `run m403_orphan_batch … examples/m403_orphan_batch/verify.sh`，
#   且该脚本文件在盘上）；否则清单一律不算数 —— 那些门照旧按孤儿判红。
#   （负控见 examples/m403_orphan_batch 的 C4/C5，以及本文件 --self-test 的 N4：清单在而消费者不在 ⇒ 仍判红。）
REVIVE="${ORPHAN_REVIVED:-selfhost/orphan_revived.txt}"
CONSUMER="m403_orphan_batch"
if [ -f "$REVIVE" ]; then
    if grep -qE "^[[:space:]]*run[[:space:]]+${CONSUMER}[[:space:]]+.*examples/${CONSUMER}/verify\.sh" "$REG" \
       && [ -f "$EX_DIR/$CONSUMER/verify.sh" ]; then
        grep -vE '^[[:space:]]*(#|$)' "$REVIVE" | tr -d '[:blank:]' | grep . | sort -u > "$T/revived.txt" || true
        cat "$T/revived.txt" >> "$T/covered.txt"
        sort -u -o "$T/covered.txt" "$T/covered.txt"
        ok "批量复活清单（$(wc -l < "$T/revived.txt") 条 · 消费者 ${CONSUMER} 已注册且脚本在盘）"
    else
        echo "  ⚠️ 复活清单存在但消费者未注册/缺脚本 ⇒ 清单不算收录（其中门仍按孤儿判）"
    fi
fi

NA=$(wc -l < "$T/all.txt")
NR=$(wc -l < "$T/reg.txt")
NC=$(wc -l < "$T/ci.txt")
echo "  实存门 $NA · registry $NR · ci.yml $NC"

comm -23 "$T/all.txt" "$T/covered.txt" > "$T/orphan.txt"
NO=$(wc -l < "$T/orphan.txt")
echo "  孤儿门（两个运行器都没收录）= $NO"

# ── 2 规模锚点（防「提取器坏了 ⇒ 空集 ⊇ 空集」的假绿）──
if [ "$FIXTURE" = 1 ]; then
    echo "  ⏭  夹具模式（ORPHAN_SELFTEST=1）：规模锚点跳过（2~3 门的小夹具达不到 150/100/40）"
else
    [ "$NA" -ge 150 ] && ok "实存门规模下限（≥150）" || bad "实存门只 $NA 个 —— find 提取器坏了？"
    [ "$NR" -ge 100 ] && ok "registry 规模下限（≥100）" || bad "registry 只 $NR 个 —— 提取器坏了？"
    [ "$NC" -ge 100 ] && ok "ci.yml 规模下限（≥100）" || bad "ci.yml 只 $NC 个 —— 提取器坏了？"
fi

# ── 3 基线表 ──
if [ ! -f "$BASE" ]; then
    bad "基线表不存在：$BASE（建它，或把孤儿清零）"
else
    grep -vE '^[[:space:]]*(#|$)' "$BASE" | cut -d'|' -f1 | sed 's/[[:space:]]*$//' | sort -u > "$T/base.txt"
    NB=$(wc -l < "$T/base.txt")

    # 3a 新增孤儿：在孤儿集、不在基线 ⇒ **判红**（债不许扩大）
    comm -23 "$T/orphan.txt" "$T/base.txt" > "$T/new.txt"
    NN=$(wc -l < "$T/new.txt")
    if [ "$NN" = 0 ]; then
        ok "无新增孤儿（$NO 个孤儿全部已登记）"
    else
        bad "**新增孤儿 $NN 个**（有 verify.sh 但没有任何运行器跑它，且不在基线）:"
        sed 's/^/       /' "$T/new.txt"
    fi

    # 3b 过期基线：在基线、不在孤儿集 ⇒ **判红**（登记必须仍然成立）
    comm -13 "$T/orphan.txt" "$T/base.txt" > "$T/stale.txt"
    NS=$(wc -l < "$T/stale.txt")
    if [ "$NS" = 0 ]; then
        ok "基线无过期条目（$NB 条 · 全部仍然成立）"
    else
        bad "基线**已过期** $NS 条（它们已被收录/已不存在 ⇒ 必须从 $BASE 移除）:"
        sed 's/^/       /' "$T/stale.txt"
    fi

    # 3c 每条必须带理由（≥8 字）
    NOREASON=0
    while IFS= read -r line; do
        case "$line" in ''|'#'*) continue ;; esac
        nm="$(printf '%s' "$line" | cut -d'|' -f1 | sed 's/[[:space:]]*$//')"
        rs="$(printf '%s' "$line" | cut -s -d'|' -f2-)"
        [ -n "$nm" ] || continue
        if [ -z "$rs" ] || [ "${#rs}" -lt 8 ]; then
            echo "       ⚠️ 缺理由/理由过短: $nm"
            NOREASON=$((NOREASON+1))
        fi
    done < <(grep -vE '^[[:space:]]*(#|$)' "$BASE")
    [ "$NOREASON" = 0 ] && ok "基线条目全部带理由（≥8 字）" || bad "$NOREASON 条基线条目缺理由"

    # ── 3e 体检台账（M251 · 缺陷 428 的「下半条」）─────────────────────
    # M249 挡住了「新增孤儿」，但基线理由统一是「存量」——
    #   「这扇门现在还是绿的吗？它依赖的东西还在吗？」**无人回答**。
    #   m63 已证明「孤儿门 = 判据腐烂的温床」⇒ 体检必须留下**可核对**的记录。
    AUDIT="${ORPHAN_AUDIT:-selfhost/orphan_audit.tsv}"
    if [ ! -f "$AUDIT" ]; then
        bad "体检台账不存在：$AUDIT（每个孤儿门都要有一条实测记录）"
    else
        awk -F'\t' '!/^#/ && NF>=5 {print $1}' "$AUDIT" | sed 's/[[:space:]]*$//' | sort -u > "$T/audit.txt"
        NX=$(wc -l < "$T/audit.txt")
        echo "  体检台账 $NX 条"
        comm -23 "$T/base.txt" "$T/audit.txt" > "$T/a_miss.txt"
        comm -13 "$T/base.txt" "$T/audit.txt" > "$T/a_stale.txt"
        NM=$(wc -l < "$T/a_miss.txt"); NX2=$(wc -l < "$T/a_stale.txt")
        [ "$NM" = 0 ] && ok "台账无漏记（$NB 个孤儿门全部有实测记录）" \
                      || { bad "台账**漏记 $NM 个**（孤儿门没有实测记录）:"; sed 's/^/       /' "$T/a_miss.txt"; }
        [ "$NX2" = 0 ] && ok "台账无过期条目" \
                       || { bad "台账**过期 $NX2 条**（不在基线里 ⇒ 删它）:"; sed 's/^/       /' "$T/a_stale.txt"; }

        # 3e-2 分类合法 + 非 OK 条目必须带定性处置
        BADCLS=0; BADNOTE=0
        : > "$T/a_bad.txt"
        while IFS=$'\t' read -r g deps rc sec cls note; do
            case "$g" in ''|'#'*) continue ;; esac
            case "$cls" in
                OK|SKIP|FAIL|TIMEOUT) ;;
                *) echo "       ⚠️ 非法分类 '$cls'（门 $g）"; BADCLS=$((BADCLS+1)) ;;
            esac
            case "$cls" in
                SKIP|FAIL|TIMEOUT)
                    case "$note" in
                        *已修*|*已登记*|*环境*|*设计性*) ;;
                        *) echo "       ⚠️ $g 分类 $cls 但无处置（note 需含 已修/已登记/环境/设计性）"; BADNOTE=$((BADNOTE+1)) ;;
                    esac ;;
            esac
        done < "$AUDIT"
        [ "$BADCLS" = 0 ] && ok "台账分类字段全部合法" || bad "$BADCLS 条分类非法"
        [ "$BADNOTE" = 0 ] && ok "非 OK 条目全部带定性处置" || bad "$BADNOTE 条缺处置文本"

        # 3e-3 规模锚点（防「空台账 ⊇ 空基线」的假绿）
        if [ "$FIXTURE" = 1 ]; then
            echo "  ⏭  夹具模式：台账规模锚点跳过"
        else
            # M403：孤儿基线从 46 → 7（39 扇复活、进 orphan_revived.txt 由 m403_orphan_batch 跑），
            #   故台账规模锚点同步下调到 ≥5 —— 锚点只防「空台账 ⊇ 空基线」的假绿，
            #   真正的一一对应由上面 3e 的 comm 双向差集保证（漏记/过期都判红）。
            [ "$NX" -ge 5 ] && ok "台账规模下限（≥5，实测 $NX）" || bad "台账只 $NX 条 —— 体检没跑？"
        fi
    fi

    # 3d 基线只许缩小
    if [ "$NB" -le "$ORPHAN_MAX" ]; then
        ok "基线规模 $NB ≤ 上限 $ORPHAN_MAX（只许缩小）"
    else
        bad "基线规模 $NB > 上限 $ORPHAN_MAX —— 债在扩大（要扩充请显式改 ORPHAN_MAX 并说明）"
    fi
fi

echo
if [ "$FAIL" = 0 ]; then
    echo "══ ORPHAN-GATES-OK（通过 $PASS / 失败 0）"
    exit 0
else
    echo "══ ORPHAN-GATES-FAIL（通过 $PASS / 失败 $FAIL）"
    exit 1
fi
