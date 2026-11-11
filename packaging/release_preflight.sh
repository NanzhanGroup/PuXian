#!/usr/bin/env bash
# ============================================================
# release_preflight.sh —— 发布前置「版本位」检查（M377 事故的直接产物）
# ------------------------------------------------------------
# 事故（2026-11-06）：
#   m377_chain_wrap.sh 跑完 253 门（2h05m，全绿）后直接调 make_tag.sh，
#   而源码版本串还停在 0.2.376 ⇒ 闸门（缺陷 638）拒绝：rc=3 · tag=无。
#   代价：2 小时门跑完才知道「版本位没抬」，白等一轮。
#
# 本脚本只做一件事：把「tag 段（里程碑号）== 源码版本串 patch 段」这一条
#   在**跑门之前**用 1 秒钟判掉。
#
# 判据来源（都不写死数字）：
#   里程碑  = CHANGELOG.md 第一个二级标题 "## M<num>：" 的 num
#   版本串  = tools/px 里 ^SELFHOST_VER="X.Y.Z"
#   一致    = X.Y.Z 的 Z == num
#
# 用法：release_preflight.sh [--root <仓库根>] [--quiet]
#       release_preflight.sh --gates          # 提交前门档（见下）
#       release_preflight.sh --self-test      # 正控 + 负控（含前提自证）
# 退出码：0 = 一致，可以开跑 · 3 = 版本位未抬（附修法）· 2 = 参数/环境错
#         4 = --gates 档有门判红 · 1 = 自测失败
#
# --gates 档（2026-11-11 用户令「做提交前跑」）：
#   M390 单日白跑两轮全量链（r1 92min 判 12 红 · r3 92min 判 2 红），红的**全部**是
#   登记类判据 —— 域前缀闭集、三轨对拍、登记表⇄磁盘。这些门自己跑只要 30 秒~12 分钟。
#   用 92 分钟的反馈周期去发现 30 秒能发现的问题，就是这一档要消除的东西。
#   顺序：commit（树净）→ release_preflight.sh --gates → 起全量链
# ============================================================
set -uo pipefail

ROOT=""; QUIET=0; SELF=0; GATES=0
while [ $# -gt 0 ]; do
    case "$1" in
        --root) ROOT="${2:-}"; shift 2 ;;
        --quiet) QUIET=1; shift ;;
        --self-test) SELF=1; shift ;;
        --gates) GATES=1; shift ;;
        -h|--help) sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done

check() {  # check <root> → 打印结论，rc 0/3/2
    local r="$1" ms ver p z
    [ -f "$r/CHANGELOG.md" ] || { echo "❌ 缺 CHANGELOG.md：$r" >&2; return 2; }
    [ -f "$r/tools/px" ]     || { echo "❌ 缺 tools/px：$r" >&2; return 2; }

    ms="$(grep -m1 -oE '^## M[0-9]+' "$r/CHANGELOG.md" 2>/dev/null | grep -oE '[0-9]+' || true)"
    ver="$(grep -m1 -oE '^SELFHOST_VER="[0-9]+\.[0-9]+\.[0-9]+"' "$r/tools/px" 2>/dev/null | sed 's/^SELFHOST_VER="//; s/"$//' || true)"

    # 判据必须「取得到」才算数 —— 取不到一律 rc=2，绝不静默当绿（缺陷 288 的纪律）
    [ -n "$ms" ]  || { echo "❌ CHANGELOG.md 顶部没有 '## M<数字>：' 标题行（GATE_7 要求每轮必须有）" >&2; return 2; }
    [ -n "$ver" ] || { echo "❌ tools/px 里读不到 SELFHOST_VER=\"X.Y.Z\"" >&2; return 2; }

    p="$(echo "$ver" | awk -F. '{print $3}')"
    if [ "$p" = "$ms" ]; then
        [ "$QUIET" = 1 ] || echo "VERSION-POSITION=OK 源码=$ver 里程碑=M$ms"
        return 0
    fi
    echo "VERSION-POSITION=STALE 源码=$ver 里程碑=M$ms 期望=0.2.$ms" >&2
    echo "   修法（顺序固定，别跳步）：" >&2
    echo "     ./packaging/bump_version.sh X.Y.$ms          # 改 9 处字面量" >&2
    echo "     ./selfhost/rebake_bin.sh --rebake-all        # 必须 --rebake-all，否则 pxi/pxfmt/pxlsp/pxmcp 仍印旧版本" >&2
    echo "     ./selfhost/rebake_bin.sh --check-all         # 期望 14/14" >&2
    echo "     bash selfhost/check_version_golden.sh && bash selfhost/check_version_drift.sh && ./selfhost/bootstrap_prove.sh" >&2
    echo "     git commit → packaging/make_tag.sh --milestone $ms --push" >&2
    return 3
}

# ── 提交前门档（--gates）─────────────────────────────────────
# 清单只写门名，**不写路径**：每个名字必须在 selfhost/gates.registry.sh 里以
#   `run <name> ` 存在，否则 rc=2。名字打错 = 静默跳过 = 假绿，故按环境错处理。
# runner 可注入（PREFLIGHT_RUN_GATES）：自测用它换成 stub，绝不跑真门。
PRECHECK_GATES=(
    m191_error_codes m192_encfamily m193_errcodes2   # 错误码域前缀闭集
    m196_msg_parity m197_builtin_arity               # 三轨 builtin 对拍
    m199_argtype                                     # 负控锚点有效性
    m282_registry_020 m187_registry_import           # 登记表 ⇄ 磁盘（m187 约 700s，本档最慢的一门）
    version_drift version_golden                     # 版本字面量 9 处 + 自举基准（M392 首跑 3 红里的 2 红，几秒一门）
    fmt_check                                        # pxfmt 规范（M391 首跑红之一，20s）
)

gate_check() {  # gate_check <root> → 0 全绿 / 4 有红 / 2 环境错
    local r="$1" reg="$1/selfhost/gates.registry.sh" g rc log tsv rows t0 missing=0

    [ -f "$reg" ] || { echo "❌ 缺 registry：$reg" >&2; return 2; }

    # ① 存在性自证：名字不在 registry ⇒ 环境错，绝不「跳过继续」
    for g in "${PRECHECK_GATES[@]}"; do
        grep -qE "^run ${g} " "$reg" || { echo "❌ 门名不在 registry：$g（打错即静默跳过 ⇒ 按环境错）" >&2; missing=1; }
    done
    [ "$missing" = 0 ] || return 2

    # ② 树必须干净：门跑期间源码不可变，否则「绿」对应的是另一份代码
    if [ -n "$(git -C "$r" status --porcelain 2>/dev/null)" ]; then
        echo "❌ 工作树不干净 ⇒ --gates 拒绝跑（先 commit，再跑本档；这也是 publish 的树净锁）" >&2
        return 2
    fi

    local runner="${PREFLIGHT_RUN_GATES:-$r/selfhost/run_gates.sh}"
    { [ -x "$runner" ] || [ -f "$runner" ]; } || { echo "❌ 找不到门运行器：$runner" >&2; return 2; }

    t0=$SECONDS
    log="$(mktemp)"
    tsv="$log.tsv"
    # 不用管道接 runner：rc 必须是门自己的 rc（缺陷 696 的形状：管道吞掉对侧状态）
    # GATE_TSV 指到临时文件：下面要用「台账行数」证明**这几门真的跑了**，
    #   因为实测 run_gates.sh 对不存在的门名会 rc=0 + 「跳过 267 门」⇒ rc=0 不足以证明跑过。
    GATE_TSV="$tsv" bash "$runner" --only "$(IFS=,; echo "${PRECHECK_GATES[*]}")" >"$log" 2>&1
    rc=$?
    tail -30 "$log"
    echo "PREFLIGHT-GATES $( [ "$rc" = 0 ] && echo OK || echo FAIL ) rc=$rc 门数=${#PRECHECK_GATES[@]} 耗时=$((SECONDS-t0))s"
    if [ "$rc" != 0 ]; then
        echo "   红门明细：" >&2
        grep -E '❌|^FAIL|✗' "$log" | head -20 >&2
        rm -f "$log" "$tsv"
        return 4
    fi
    rows="$(wc -l < "$tsv" 2>/dev/null || echo 0)"
    rows="${rows//[!0-9]/}"
    if [ "${rows:-0}" -lt "${#PRECHECK_GATES[@]}" ]; then
        echo "❌ 台账只有 ${rows:-0} 行 / 清单 ${#PRECHECK_GATES[@]} 门 ⇒ 这几门没真跑起来（rc=0 不算绿）" >&2
        rm -f "$log" "$tsv"
        return 2
    fi
    # ④ 结束前**再判一次**树净（M391 实测补上）：
    #   2026-11-11 15:29 首跑时我在门跑的过程中改了 selfhost/*.px（开发三元表达式），
    #   开始时的树净自证已经过了 ⇒ 这次「绿」对应的是**半路被换掉的代码**，结论作废。
    #   判据：跑完时工作树必须仍与开始时一致（有改动 ⇒ 测量期间被测物变了 ⇒ rc=2 无效测量）。
    if [ -n "$(git -C "$r" status --porcelain 2>/dev/null)" ]; then
        echo "❌ 门跑期间工作树被改动 ⇒ 本次结果**无效**（不是红、也不是绿）：" >&2
        git -C "$r" status --porcelain 2>/dev/null | head -10 | sed 's/^/     /' >&2
        rm -f "$log" "$tsv"
        return 2
    fi
    rm -f "$log" "$tsv"
    return 0
}

if [ "$SELF" = 1 ]; then
    T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
    mk() {  # mk <dir> <milestone> <version>
        mkdir -p "$1/tools"
        printf '## M%s：夹具\n\n正文\n' "$2" > "$1/CHANGELOG.md"
        printf '#!/usr/bin/env bash\nSELFHOST_VER="%s"\n' "$3" > "$1/tools/px"
    }
    FAIL=0; n=0
    ok()  { n=$((n+1)); echo "  ✅ $*"; }
    bad() { n=$((n+1)); FAIL=$((FAIL+1)); echo "  ❌ $*"; }

    mk "$T/p1" 377 0.2.377
    if check "$T/p1" >/dev/null 2>&1; then ok "P1 正控：M377 + 0.2.377 ⇒ 绿"; else bad "P1 正控判红（应绿）"; fi

    mk "$T/n1" 378 0.2.377
    out="$(check "$T/n1" 2>&1)"; rc=$?
    [ "$rc" = 3 ] && ok "N1 负控：里程碑抬到 M378 而版本串仍是 0.2.377 ⇒ rc=3（正是 M377 事故现场）" \
                  || bad "N1 负控 rc=$rc（应 3）"
    # 前提自证：证明扰动真的发生了（挂载点里 milestone 确实是 378）
    grep -q '^## M378' "$T/n1/CHANGELOG.md" && ok "N1 前提自证：夹具 CHANGELOG 顶部确为 M378" \
                                           || bad "N1 前提自证失败（扰动没发生，判据无效）"
    grep -q '期望=0.2.378' <<< "$out" && ok "N1 报出正确的期望版本 0.2.378" || bad "N1 没报出期望版本"

    mk "$T/n2" 379 0.2.379; sed -i 's/^## M379/## 没有里程碑行/' "$T/n2/CHANGELOG.md"
    rc=0; check "$T/n2" >/dev/null 2>&1 || rc=$?
    [ "$rc" = 2 ] && ok "N2 负控：CHANGELOG 无标题行 ⇒ rc=2（取不到就红，不当绿）" \
                  || bad "N2 负控 rc=$rc（应 2）"

    # ── --gates 档：1 正控 + 4 负控 + 3 前提自证（stub runner，绝不跑真门）──
    gmkrepo() {  # gmkrepo <dir> [skip_gate] [dirty]
        mkdir -p "$1/selfhost"
        { for g in "${PRECHECK_GATES[@]}"; do [ "$g" = "${2:-}" ] || echo "run $g bash x"; done; } \
            > "$1/selfhost/gates.registry.sh"
        cat > "$1/selfhost/run_gates.sh" <<'STUB'
#!/usr/bin/env bash
echo "stub-called $*" >> "$STUB_MARKER"
for i in $(seq 1 "${STUB_ROWS:-8}"); do printf 'g%s\tPASS\t1\t0\n' "$i" >> "${GATE_TSV:-/dev/null}"; done
exit "${STUB_RC:-0}"
STUB
        chmod +x "$1/selfhost/run_gates.sh"
        ( cd "$1" && git init -q 2>/dev/null && git add -A 2>/dev/null \
              && git -c user.email=a@b -c user.name=t commit -qm x 2>/dev/null )
        [ "${3:-0}" = 1 ] && echo junk > "$1/junk.txt"
        return 0
    }
    ga() {  # ga <dir> <stub_rc> <stub_rows> <outfile> → 打印 gate_check 的 rc
        STUB_MARKER="$1.stub.used" STUB_RC="$2" STUB_ROWS="$3" \
        PREFLIGHT_RUN_GATES="$1/selfhost/run_gates.sh" \
        gate_check "$1" >"$4" 2>&1; echo $?
    }

    gmkrepo "$T/g1"
    NG="${#PRECHECK_GATES[@]}"
    rc="$(ga "$T/g1" 0 "$NG" "$T/g1.out")"
    [ "$rc" = 0 ] && ok "G-P1 正控：$NG 门全绿 + 台账 $NG 行 ⇒ rc=0" || bad "G-P1 rc=$rc（应 0）"
    grep -q 'PREFLIGHT-GATES OK' "$T/g1.out" && ok "G-P1 汇总行 = OK" || bad "G-P1 缺 OK 汇总行"
    grep -q 'stub-called' "$T/g1.stub.used" 2>/dev/null && ok "G-P1 前提自证：stub runner 真被调用" \
                                        || bad "G-P1 前提缺失：stub 没被调用（判据不成立）"

    gmkrepo "$T/g2"
    rc="$(ga "$T/g2" 1 "$NG" "$T/g2.out")"
    [ "$rc" = 4 ] && ok "G-N1 负控：有一门判红 ⇒ rc=4" || bad "G-N1 rc=$rc（应 4）"

    gmkrepo "$T/g3" m187_registry_import
    rc="$(ga "$T/g3" 0 "$NG" "$T/g3.out")"
    [ "$rc" = 2 ] && ok "G-N2 负控：门名不在 registry ⇒ rc=2（不许静默跳过当绿）" || bad "G-N2 rc=$rc（应 2）"
    grep -q '门名不在 registry：m187_registry_import' "$T/g3.out" && ok "G-N2 点名了缺失的那门" || bad "G-N2 没点名"

    gmkrepo "$T/g4" "" 1
    [ -n "$(git -C "$T/g4" status --porcelain 2>/dev/null)" ] && ok "G-N3 前提自证：假 repo 确实是脏的" \
                                                              || bad "G-N3 前提缺失：假 repo 不脏"
    rc="$(ga "$T/g4" 0 "$NG" "$T/g4.out")"
    [ "$rc" = 2 ] && ok "G-N3 负控：树脏 ⇒ rc=2（门全绿也不许过）" || bad "G-N3 rc=$rc（应 2）"

    # 实测 run_gates.sh 对不存在的门名回 rc=0 + 「跳过 267 门」⇒ rc=0 不足以证明跑过。
    # 这一条负控就是钉住那个前提：stub 回 0 但台账 0 行，必须判环境错而不是绿。
    gmkrepo "$T/g5"
    rc="$(ga "$T/g5" 0 0 "$T/g5.out")"
    [ "$rc" = 2 ] && ok "G-N4 负控：rc=0 但台账 0 行（这几门没真跑）⇒ rc=2" || bad "G-N4 rc=$rc（应 2）"

    # G-N5（M391 实测补）：**门跑期间**树被改动 ⇒ 本次结果无效 ⇒ rc=2。
    #   现场：2026-11-11 15:29 首跑时我在跑的过程中改 selfhost/*.px（开发三元表达式），
    #   开始时的树净自证已过 ⇒ 「绿」对应的是半路被换掉的代码。
    #   stub：开跑前树净，跑的时候自己新建一个文件（模拟「测量期间被测物变了」）。
    gmkrepo "$T/g6"
    cat > "$T/g6/selfhost/run_gates.sh" <<'STUB2'
#!/usr/bin/env bash
echo "late-edit" > "$STUB_REPO/late.txt"      # 跑的过程中改动工作树
for i in $(seq 1 "${STUB_ROWS:-8}"); do printf 'g%s\tPASS\t1\t0\n' "$i" >> "${GATE_TSV:-/dev/null}"; done
exit 0
STUB2
    chmod +x "$T/g6/selfhost/run_gates.sh"
    ( cd "$T/g6" && git add -A 2>/dev/null && git -c user.email=a@b -c user.name=t commit -qm runner 2>/dev/null )
    rc="$(STUB_REPO="$T/g6" STUB_ROWS="$NG" PREFLIGHT_RUN_GATES="$T/g6/selfhost/run_gates.sh" \
          gate_check "$T/g6" >"$T/g6.out" 2>&1; echo $?)"
    [ "$rc" = 2 ] && ok "G-N5 负控：门跑期间树被改动 ⇒ rc=2（无效测量，不当绿）" || bad "G-N5 rc=$rc（应 2）"
    grep -q '门跑期间工作树被改动' "$T/g6.out" && ok "G-N5 报出了「无效测量」而不是含糊失败" || bad "G-N5 文案不对"
    [ -f "$T/g6/late.txt" ] && ok "G-N5 前提自证：扰动（跑期间的改动）确实发生了" || bad "G-N5 前提缺失"

    echo "── 自测：$((n-FAIL))/$n 通过 ──"
    [ "$FAIL" = 0 ] || exit 1
    exit 0
fi

if [ -z "$ROOT" ]; then
    ROOT="$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null || true)"
    [ -n "$ROOT" ] || ROOT="/data/code/puxian"
fi
if [ "$GATES" = 1 ]; then gate_check "$ROOT"; exit $?; fi
check "$ROOT"
