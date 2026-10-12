#!/usr/bin/env bash
# ============================================================
# check_no_selfwake.sh —— 「禁止自建平行告警/唤醒通道」守卫
#                        （用户令 2026-10-12 · 缺陷 517/518 的机械化护栏）
# ------------------------------------------------------------
# 为什么有这道门：
#   2026-10-11 我自建了 dy-wake.sh（往 ws-push-qq.sock 写 msg_type:inject
#   = **伪造一条「管理员本人」的 QQ 消息**）来叫醒自己。它绕开了 ws-core 的
#   空闲门禁（busWakeFilterSelfMode 只覆盖总线路径；从 QQ 网关注入的消息
#   ws-core 看不见）⇒ 一次事故**叫醒两个会话** ⇒ 两个东月同抢一个仓库、
#   实测互踩（M299 / M304 各一次）。
#   用户令：告警/唤醒一律走**系统原生通道**（dy-notify.sh → ws-core 任务自驱），
#           **禁止自建平行通道**。
#   为什么必须机械化：清记忆解决不了 —— 脚本在磁盘上，而全局规则
#   [MONITORING/P0]「必须自带通知机制」**每次请求都在把人推向「再造一个」**。
#   ⇒ 规则要**不靠记性**：本门就是那条护栏（同 2026-10-12 新增的
#     references_global.yaml [NO_SELFCHANNEL/P0]）。
#
# 判据（四类）：
#   J1 仓库面：**非注释代码行**含 dy-wake / dy_wake ⇒ 必须在豁免表
#              （selfhost/selfwake_allow.tsv）登记，且**计数不得超上限**。
#              表**双向核对**（M287 纪律）：实测有 / 表里无 ⇒ 漏登记；
#              表里有 / 实测无 ⇒ 登记过期。两者都判红。
#   J2 无人值守链面（**零豁免**）：packaging/chain/ 与 /data/gatevm/ 下的脚本
#              **整体**不得出现 dy-wake（非注释行）。豁免表**不适用**于本档
#              —— 这两处就是「事故发生时真的会去注入」的地方。
#   J3 伪造注入：非注释行同时含 msg_type 与 inject ⇒ 判红（仓库面 + 本机面）。
#   J4 规模锚点：扫描面文件数低于下限 ⇒ rc=3（防「提取器坏了 ⇒ 空集 ⊇ 空集」假绿，
#              M212 缺陷 288 / M283 缺陷 492 同族）。
#
# ⚠️ 判据只认「非注释行」：**注释里提历史说明是允许的**（先例 m255 判据
#    「代码行里 gethostbyname = 0，注释保留历史说明是允许的」）。
# ⚠️ 本门**刻意不扫 .md**：散文里写「dy-wake 已退役」是**说明**不是调用；
#    且 M284 实测过「文档里的反例会被 git grep 变成正例」⇒ 排除后本判据免疫自指。
# ⚠️ 本门**跳过自身**（M213 教训：首版守卫把自己的 PATTERNS 判红了）。
#
# 用法：selfhost/check_no_selfwake.sh [--root DIR] [--local DIRS] [--self-test] [-v]
# 退出码：0 干净 · 1 有违例 · 2 用法错 · 3 判据自身失效
# 可覆盖环境变量（自证/负控用）：
#   SELFW_ALLOW      豁免表路径
#   SELFW_LOCAL_DIRS 本机扫描目录（默认 "/data/gatevm /usr/local/sbin"；置空 = 不扫）
#   SELFW_CHAIN_ABS  J2 的本机零豁免目录（默认 /data/gatevm）
#   SELFW_MIN_FILES  J4 规模下限（默认 300；自证用）
# ============================================================
set -u
ROOT=""; SELFTEST=0; VERBOSE=0
while [ $# -gt 0 ]; do
    case "$1" in
        --root)  ROOT="$2"; shift 2 ;;
        --local) SELFW_LOCAL_DIRS="$2"; shift 2 ;;
        --self-test) SELFTEST=1; shift ;;
        -v) VERBOSE=1; shift ;;
        -h|--help) sed -n '2,46p' "$0"; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done
SELF_DIR="$(cd "$(dirname "$(readlink -f "$0" 2>/dev/null || echo "$0")")" && pwd)"
[ -n "$ROOT" ] || ROOT="$(cd "$SELF_DIR/.." && pwd)"
SELF_PATH="$(readlink -f "$0" 2>/dev/null || echo "$0")"
# ⚠️ 按**基名**跳过「判据载体自身」（M213 教训：首版守卫把自己的 PATTERNS 判红了）。
#    按基名而非绝对路径：自证夹具里复制一份也叫这个名字，**两种位置都必须被跳过**。
SELF_BASE="$(basename "$SELF_PATH")"
ALLOW="${SELFW_ALLOW:-$SELF_DIR/selfwake_allow.tsv}"
LOCAL_DIRS="${SELFW_LOCAL_DIRS-/data/gatevm /usr/local/sbin}"
CHAIN_ABS="${SELFW_CHAIN_ABS-/data/gatevm}"
MIN_FILES="${SELFW_MIN_FILES:-300}"

# ── 判据原语 ────────────────────────────────────────────────
# 去注释：抹掉行尾 " #…" 与整行注释。
# ⚠️ 这是**启发式**（对含 " # " 的字符串字面量有极小误判，方向是"多报"）。
#    实测本仓命中面：4 个文件 / 20 行，逐行人工核对过（见豁免表）。
strip_c() { sed -E -e 's/[[:space:]]+#.*$//' -e '/^[[:space:]]*#/d'; }

OKN=0; BADN=0
ok()  { OKN=$((OKN+1)); printf '  ✅ %s\n' "$1"; }
bad() { BADN=$((BADN+1)); printf '  ❌ %s\n' "$1"; }
inf() { printf '  ℹ️  %s\n' "$1"; }

# 收仓库面文件（已跟踪 ∪ 未跟踪未忽略 —— M282 事故后改，别再退回只扫 git ls-files）
collect_repo() {
    git -C "$ROOT" ls-files -co --exclude-standard 2>/dev/null \
        | grep -E '\.(sh|px|py|ya?ml|bash)$' \
        | sed "s|^|$ROOT/|"
}
collect_local() {
    for d in $LOCAL_DIRS; do
        [ -d "$d" ] || continue
        # ⚠️ **只扫顶层**（maxdepth 1）：这些目录下的 `tmp*/` 是跑门时的工作目录，
        #    里面的 `m*_stub.py` 之类是**门的临时桩**，与「无人值守链」无关。
        #    实测：写成 maxdepth 2 会把分片留下的 /data/gatevm/tmp2/m132_stub.py
        #    算进扫描面 ⇒ 计数随「有没有刚跑过门」浮动（假阳源）。
        find "$d" -maxdepth 1 -type f \( -name '*.sh' -o -name '*.py' \) 2>/dev/null
    done
}

# 每个文件「非注释行含 dy-wake」的处数
count_wake() { strip_c < "$1" 2>/dev/null | grep -c 'dy-wake\|dy_wake' || true; }
# ⚠️ 判据形状**运行时拼装**（M282 纪律：源码里不出现连续的禁止形状 ⇒ 守卫不会自指）。
#    下面两行**各自只含一半**，且 `grep -E` 逐行匹配 ⇒ 守卫自身那一行永不命中。
#    （若写成一整行 `K_MT='msg_type'; K_INJ='inject'` 就**会**自命中 —— 实测踩过。）
K_MT='msg_type'
K_INJ='inject'
PAT_INJ="${K_MT}.*${K_INJ}|${K_INJ}.*${K_MT}"
count_inject() { strip_c < "$1" 2>/dev/null | grep -cE "$PAT_INJ" || true; }

# ── 自证 ────────────────────────────────────────────────────
if [ "$SELFTEST" = 1 ]; then
    W="$(mktemp -d "${TMPDIR:-/tmp}/selfw_st.XXXXXX")"
    trap 'rm -rf "$W"' EXIT
    mk() {
        # ⚠️ 夹具必须是**真 git 仓库** —— 仓库面用的是 `git ls-files -co --exclude-standard`
        #    （含未跟踪未忽略，M282 教训）；非 git 目录会收到 0 文件 ⇒ 自证会以为判据坏了。
        mkdir -p "$W/repo/selfhost" "$W/repo/packaging/chain"
        git -C "$W/repo" init -q 2>/dev/null
        git -C "$W/repo" config user.email x@x 2>/dev/null
        git -C "$W/repo" config user.name x 2>/dev/null
    }
    run() { SELFW_LOCAL_DIRS="" SELFW_CHAIN_ABS="$W/none" SELFW_ALLOW="$2" SELFW_MIN_FILES="${3:-1}" \
            bash "$SELF_PATH" --root "$W/repo" >"$W/out.txt" 2>&1; echo $?; }

    printf '\n══ 自证 ══\n'
    # S1 干净 ⇒ rc=0
    mk
    : > "$W/allow.tsv"
    echo 'echo hello' > "$W/repo/selfhost/clean.sh"
    rc=$(run x "$W/allow.tsv"); [ "$rc" = 0 ] && ok "S1 干净树 ⇒ rc=0" || { bad "S1 干净树 ⇒ rc=$rc（期望 0）"; sed -n '1,10p' "$W/out.txt"; }

    # S2 合成「调用 dy-wake」⇒ 必须判红（负控）
    printf 'bash /usr/local/sbin/dy-wake.sh KEY "t"\n' > "$W/repo/selfhost/bad_wake.sh"
    rc=$(run x "$W/allow.tsv"); [ "$rc" = 1 ] && ok "S2 调用 dy-wake ⇒ rc=1（负控）" || { bad "S2 期望 rc=1 实得 $rc"; sed -n '1,10p' "$W/out.txt"; }
    grep -q '漏登记' "$W/out.txt" && ok "S2 报「漏登记」并指名文件" || bad "S2 未报漏登记"

    # S3 合成「伪造 inject」⇒ 必须判红（负控）
    rm -f "$W/repo/selfhost/bad_wake.sh"
    printf 'send({"%s":"%s"})   # 夹具（运行时拼装）\n' "$K_MT" "$K_INJ" > "$W/repo/selfhost/bad_inject.sh"
    rc=$(run x "$W/allow.tsv"); [ "$rc" = 1 ] && ok "S3 伪造 inject ⇒ rc=1（负控）" || { bad "S3 期望 rc=1 实得 $rc"; sed -n '1,10p' "$W/out.txt"; }

    # S4 注释里提 dy-wake ⇒ 必须**不**判红（防假阳）
    rm -f "$W/repo/selfhost/bad_inject.sh"
    printf '# 历史上用过 dy-wake.sh，已退役\necho ok\n' > "$W/repo/selfhost/comment_only.sh"
    rc=$(run x "$W/allow.tsv"); [ "$rc" = 0 ] && ok "S4 仅注释提及 ⇒ rc=0（防假阳）" || { bad "S4 期望 rc=0 实得 $rc"; sed -n '1,10p' "$W/out.txt"; }

    # S5 豁免表计数上限：18 处登记 18 ⇒ 绿；加到 19 ⇒ 红（「只减不增」有牙）
    rm -f "$W/repo/selfhost/comment_only.sh"
    { echo 'echo a'; for i in $(seq 1 18); do echo "echo dy-wake$i"; done; } > "$W/repo/selfhost/allowlisted.sh"
    printf 'selfhost/allowlisted.sh\t18\t自证夹具\n' > "$W/allow18.tsv"
    rc=$(run x "$W/allow18.tsv"); [ "$rc" = 0 ] && ok "S5a 登记 18 / 实测 18 ⇒ rc=0" || { bad "S5a 期望 rc=0 实得 $rc"; sed -n '1,10p' "$W/out.txt"; }
    echo 'echo dy-wake19' >> "$W/repo/selfhost/allowlisted.sh"
    rc=$(run x "$W/allow18.tsv"); [ "$rc" = 1 ] && grep -q '计数增加' "$W/out.txt" && ok "S5b 实测 19 > 上限 18 ⇒ rc=1 且报「计数增加」" || { bad "S5b 期望 rc=1+计数增加"; sed -n '1,10p' "$W/out.txt"; }

    # S6 登记过期 ⇒ 判红（双向核对）
    printf 'selfhost/ghost.sh\t3\t已不存在的文件\n' > "$W/allow_ghost.tsv"
    rm -f "$W/repo/selfhost/allowlisted.sh"
    rc=$(run x "$W/allow_ghost.tsv"); [ "$rc" = 1 ] && grep -q '登记过期' "$W/out.txt" && ok "S6 表里有/实测无 ⇒ rc=1 且报「登记过期」" || { bad "S6 期望 rc=1+登记过期"; sed -n '1,10p' "$W/out.txt"; }

    # S7 J2 零豁免：无人值守链里出现 dy-wake ⇒ 必须判红（且**豁免表不适用**）
    printf 'dy-wake.sh K "t"\n' > "$W/repo/packaging/chain/watch_x.sh"
    printf 'packaging/chain/watch_x.sh\t99\t自证夹具（不该被 J2 豁免）\n' > "$W/allow_j2.tsv"
    rc=$(run x "$W/allow_j2.tsv"); [ "$rc" = 1 ] && grep -q 'J2' "$W/out.txt" && ok "S7 J2 零豁免 ⇒ rc=1（豁免表对它无效）" || { bad "S7 期望 rc=1+J2"; sed -n '1,10p' "$W/out.txt"; }

    # S7b J2 **本机侧**（真实现场：/data/gatevm 那批无人值守脚本）⇒ 必须判红
    mkdir -p "$W/chain"
    printf 'bash /usr/local/sbin/dy-wake.sh K "t"\n' > "$W/chain/local_watch.sh"
    SELFW_LOCAL_DIRS="" SELFW_ALLOW="$W/allow.tsv" SELFW_MIN_FILES=1 \
        SELFW_CHAIN_ABS="$W/chain" bash "$SELF_PATH" --root "$W/repo" >"$W/out2.txt" 2>&1
    rc=$?
    [ "$rc" = 1 ] && grep -q 'J2 本机链' "$W/out2.txt" && ok "S7b J2 本机侧零豁免 ⇒ rc=1（豁免表无效）" || { bad "S7b 期望 rc=1+J2 本机链，实得 rc=$rc"; sed -n '1,12p' "$W/out2.txt"; }
    rm -rf "$W/chain"

    # S8 规模锚点：下限调高到不可能 ⇒ rc=3（判据自身失效）
    rm -f "$W/repo/packaging/chain/watch_x.sh"
    rc=$(run x "$W/allow.tsv" 999999); [ "$rc" = 3 ] && ok "S8 规模不达标 ⇒ rc=3（判据自身失效）" || { bad "S8 期望 rc=3 实得 $rc"; sed -n '1,10p' "$W/out.txt"; }
    # S9 判据载体自身被跳过（M213 教训：首版守卫把自己的 PATTERNS 判红了）
    #    判据要**窄**：只跳过同名的那份；**改个名字**塞同样的内容 ⇒ 必须仍被抓。
    rm -f "$W/repo/selfhost/bad_inject.sh" 2>/dev/null
    cp "$SELF_PATH" "$W/repo/selfhost/check_no_selfwake.sh"
    printf 'send({"%s":"%s"})\n' "$K_MT" "$K_INJ" > "$W/repo/selfhost/other_inject.sh"
    rc=$(run x "$W/allow.tsv")
    if [ "$rc" = 1 ] && grep -q 'other_inject.sh' "$W/out.txt" \
       && ! grep -q 'J1 漏登记：selfhost/check_no_selfwake.sh' "$W/out.txt"; then
        ok "S9 只跳过判据载体自身（改名的那份仍被抓）"
    else
        bad "S9 期望 rc=1 且只抓 other_inject.sh（自身被跳过），实得 rc=$rc"
        sed -n '1,14p' "$W/out.txt"
    fi
    rm -f "$W/repo/selfhost/check_no_selfwake.sh" "$W/repo/selfhost/other_inject.sh"

    printf '\n自证：%d 通过 / %d 失败\n' "$OKN" "$BADN"
    [ "$BADN" = 0 ] || exit 3
    exit 0
fi

# ── J4 规模锚点（先量，后判）────────────────────────────────
REPO_N=$(collect_repo | wc -l)
LOC_N=$(collect_local | wc -l)
if [ "$REPO_N" -lt "$MIN_FILES" ]; then
    printf '❌ J4 判据自身失效：仓库面只收到 %d 个文件（下限 %d）—— 提取器坏了，结论不可信\n' "$REPO_N" "$MIN_FILES" >&2
    exit 3
fi

# ── J1 仓库面（豁免表 · 双向核对 · 计数只减不增）────────────
printf '\n══ J1 仓库面：dy-wake 引用 ⇄ 豁免表 ══\n'
replist="$(mktemp)"; trap 'rm -f "$replist"' EXIT
collect_repo | while read -r f; do
    [ "$(basename "$f")" = "$SELF_BASE" ] && continue     # 判据载体自身（见 SELF_BASE 注释）
    n=$(count_wake "$f"); [ "$n" -gt 0 ] && printf '%s\t%s\n' "${f#$ROOT/}" "$n"
done > "$replist"

allow_max() { # $1 = relpath
    [ -f "$ALLOW" ] || { echo ""; return; }
    awk -F'\t' -v p="$1" '!/^[[:space:]]*#/ && NF>=2 && $1==p {print $2; exit}' "$ALLOW"
}
J1_HIT=0
while IFS=$'\t' read -r rel n; do
    [ -z "${rel:-}" ] && continue
    J1_HIT=$((J1_HIT+1))
    max="$(allow_max "$rel")"
    if [ -z "$max" ]; then
        bad "J1 漏登记：$rel 有 $n 处 dy-wake（非注释行），豁免表无此条目"
    elif ! [ "$n" -le "$max" ] 2>/dev/null; then
        bad "J1 计数增加：$rel 实测 $n 处 > 豁免上限 $max（豁免只减不增）"
    else
        ok "J1 已豁免：$rel  $n/$max"
    fi
done < "$replist"
[ "$J1_HIT" = 0 ] && ok "J1 仓库面零命中"

if [ ! -f "$ALLOW" ]; then
    bad "J1 豁免表不存在：$ALLOW（不建表 ⇒ 有命中时无从登记，判据不可用）"
else
    EXPIRED=0
    while IFS=$'\t' read -r p m rest; do
        [ -z "${p:-}" ] && continue
        case "$p" in \#*) continue ;; esac
        if ! cut -f1 "$replist" | grep -qxF "$p"; then
            bad "J1 登记过期：$p 在豁免表里，但实测 0 处 ⇒ 表未跟上现实（应删除该行）"
            EXPIRED=$((EXPIRED+1))
        fi
    done < "$ALLOW"
    [ "$EXPIRED" = 0 ] && ok "J1 豁免表双向一致（无过期条目）"
fi

# ── J2 无人值守链面（零豁免）───────────────────────────────
printf '\n══ J2 无人值守链面（零豁免）══\n'
J2_HIT=0
for f in "$ROOT"/packaging/chain/*.sh; do
    [ -f "$f" ] || continue
    n=$(count_wake "$f")
    [ "$n" -gt 0 ] && { bad "J2 仓库链 ${f#$ROOT/} 引用 dy-wake（$n 处）—— 无人值守链零豁免"; J2_HIT=$((J2_HIT+1)); }
done
for d in $CHAIN_ABS; do
    [ -d "$d" ] || continue
    for f in "$d"/*.sh; do
        [ -f "$f" ] || continue
        n=$(count_wake "$f")
        [ "$n" -gt 0 ] && { bad "J2 本机链 $f 引用 dy-wake（$n 处）—— 无人值守链零豁免"; J2_HIT=$((J2_HIT+1)); }
    done
done
if [ "$J2_HIT" = 0 ]; then
    if [ -d "$CHAIN_ABS" ] || [ -d "$ROOT/packaging/chain" ]; then
        ok "J2 无人值守链零命中"
    else
        inf "J2 扫描面不存在（本机路径 ${CHAIN_ABS}）⇒ 跳过，判据未失效"
    fi
fi

# ── J3 伪造注入（msg_type + inject）────────────────────────
printf '\n══ J3 伪造注入（msg_type + inject）══\n'
J3_HIT=0
scan_inject() { # $1 = 文件（绝对路径）· $2 = 显示名
    case "$(basename "$1")" in
        dy-wake.sh) return ;;        # 退役留档自身豁免（它就是那个组件）
        "$SELF_BASE") return ;;      # 判据载体自身（同 J1，见 SELF_BASE 注释）
    esac
    n=$(count_inject "$1")
    [ "$n" -gt 0 ] && { bad "J3 $2 含 msg_type+inject（$n 处）—— 伪造注入"; J3_HIT=$((J3_HIT+1)); }
}
while read -r f; do [ -n "$f" ] && scan_inject "$f" "${f#$ROOT/}"; done < <(collect_repo)
while read -r f; do [ -n "$f" ] && scan_inject "$f" "$f"; done < <(collect_local)
if [ "$J3_HIT" = 0 ]; then ok "J3 零命中（仓库面 $REPO_N 文件 · 本机面 $LOC_N 文件）"; fi

# ── 汇总 ───────────────────────────────────────────────────
printf '\n汇总：通过 %d · 失败 %d · 扫描面 仓库 %d 文件 / 本机 %d 文件\n' "$OKN" "$BADN" "$REPO_N" "$LOC_N"
if [ "$BADN" -gt 0 ]; then
    printf '❌ SELFW-VERIFY-FAIL —— 存在自建平行通道引用。\n' >&2
    printf '   ⇒ 唯一合法路径：dy-notify.sh（→ ws-core 任务自驱）。详见 references_global.yaml [NO_SELFCHANNEL/P0]。\n' >&2
    exit 1
fi
printf 'SELFW-VERIFY-OK\n'
exit 0
