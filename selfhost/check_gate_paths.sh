#!/usr/bin/env bash
# ============================================================
# selfhost/check_gate_paths.sh —— **门里不许出现开发机绝对路径**（M213 · 缺陷 300；M317 扩面）
# ------------------------------------------------------------
# 为什么必须有它（2026-09-26 实锤，白跑两轮 CI）：
#   `examples/m213_gcroot_precision/verify.sh` 的 ⑦ 层把
#       `/data/code/puxian/selfhost/gcroot_derive.py`
#   写进了 python 代码 ⇒ CI 的仓库在 `/home/runner/work/PuXian/PuXian` ⇒
#   `FileNotFoundError` ⇒ 门红 ⇒ 「工具链自举质量门」red。
#   `examples/m198_argface/verify.sh` 的 NC-D 有同族写死（CI 用 `--neg-skip` 才没露）。
#
# ⚠️ 为什么「干净导出复现」抓不到：
#   把 `git archive` 导到 /tmp 里跑，这个**写死的路径在本机依然存在** ⇒ 本机全绿、CI 红。
#   ⇒ 只能靠 **CI** 或**这道静态守卫**。这就是本脚本存在的唯一理由。
#
# ⚠️⚠️ M317 扩面（2026-10-21 · 分片档实测）：原扫描面只有
#   `examples/*/verify.sh` + `selfhost/*.sh` + `tools/*.sh` + workflows
#   ⇒ 门目录里的 **`.py` 辅助脚本**与**子目录 `.sh`** 整类不在判据面内。后果不是「CI 红」而是
#   **分片档的结论不可信**：`examples/m316_http_dup_host/negctl.py` 写死
#   `/data/code/puxian/runtime/runtime.c` ⇒ 在分片 worktree 里跑门时**打桩改的是主仓库**，
#   被测树没变 ⇒ ① 负控自证「两档产物同 sha」判红 ② 打桩窗口内其他分片读到被污染的源码。
#   ⇒ 扫描面扩到 `.py` 与子目录 `.sh`；扩面暴露的**存量**登记在 `gate_paths_allow.tsv`。
#
# 判据：扫
#   examples/*/verify.sh · examples/*/*.py · examples/*/*.sh · selfhost/*.sh · selfhost/*.py
#   tools/*.sh · tools/*.py · packaging/*.sh · packaging/chain/*.sh · .github/workflows/*.yml
#   逐行找 `/data/code/` 与写死的 CI 工作区前缀 `/home/runner/work/`：
#     · **代码行**（首个非空白字符不是 `#`）⇒ **违例**（判红）
#     · **注释行** ⇒ 允许（m156/m157 就用注释**记载这个反模式**，是有价值的留证）
#     · 命中 `gate_paths_allow.tsv` 的文件 ⇒ 跳过并计数；**表里有、实测无该文件的违例** ⇒ 判红（过期）
# 自带自证（4 正/负 + 2 条豁免判据），失败即红。
# 用法：bash selfhost/check_gate_paths.sh [--self-test]
# 退出码：0 无违例 / 1 有违例（含豁免表过期）/ 2 自证失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
ALLOW="$HERE/gate_paths_allow.tsv"

PATTERNS=('/data/code/' '/home/runner/work/')

# 门与脚本清单（去重：`examples/*/*.sh` 与 `examples/*/verify.sh` 有交集）
gate_files() {   # $1 = 仓库根
    ls "$1"/examples/*/verify.sh "$1"/examples/*/*.py "$1"/examples/*/*.sh \
       "$1"/selfhost/*.sh "$1"/selfhost/*.py "$1"/tools/*.sh "$1"/tools/*.py \
       "$1"/packaging/*.sh "$1"/packaging/chain/*.sh "$1"/.github/workflows/*.yml 2>/dev/null \
        | sort -u
}

scan() {   # $1 = 仓库根；输出违例行 file:lineno:content
    local root="$1" f line no n=0 rel
    while IFS= read -r f; do
        [ -f "$f" ] || continue
        # ⚠️ **跳过本脚本自己**：它的 PATTERNS 与自证 fixture 里**按定义**就含这两个前缀
        #   ⇒ 不跳过的话，判据会**把自己判红**（首版实测：5 条"违例"全是它自己）。
        case "$(basename "$f")" in check_gate_paths.sh) continue ;; esac
        rel="${f#"$root"/}"
        # 豁免表（只减不增 · 每条带理由）
        if [ -f "${2:-/dev/null}" ] && cut -f1 "${2:-/dev/null}" 2>/dev/null | grep -qxF "$rel"; then
            continue
        fi
        no=0
        while IFS= read -r line; do
            no=$((no + 1))
            local body="${line#"${line%%[![:space:]]*}"}"     # 去前导空白
            case "$body" in '#'*) continue ;; esac            # 注释行允许
            for p in "${PATTERNS[@]}"; do
                case "$line" in
                    *"$p"*) echo "$f:$no:$line"; n=$((n + 1)) ;;
                esac
            done
        done < "$f"
    done < <(gate_files "$root")
    return 0
}

if [ "${1:-}" = "--self-test" ]; then
    T="$(mktemp -d /tmp/gatepaths.XXXXXX)"
    mkdir -p "$T/examples/fake" "$T/selfhost" "$T/tools" "$T/.github/workflows"
    # ① 代码行写死 ⇒ 必被抓
    printf '%s\n' 'python3 - "$PWD" <<PY' 'p = "/data/code/puxian/runtime/runtime.c"' > "$T/examples/fake/verify.sh"
    bad1="$(scan "$T" | wc -l)"
    # ② 注释行记载 ⇒ 不抓
    printf '%s\n' '# 反例：/data/code/puxian/runtime/runtime.c 不许写死' 'echo ok' > "$T/examples/fake/verify.sh"
    bad2="$(scan "$T" | wc -l)"
    # ③ CI 工作区前缀写死 ⇒ 必被抓
    printf '%s\n' 'cd /home/runner/work/PuXian/PuXian' > "$T/tools/x.sh"
    bad3="$(scan "$T" | wc -l)"
    # ④ **M317 新增：`.py` 辅助脚本里写死 ⇒ 必被抓**（原扫描面的整类缺口）
    #    ⚠️ 必须先清掉 ③ 的 fixture（tools/x.sh 也含违例）—— 否则本层数到 2 条、
    #       下一层 ⑤ 也数到 1 条 ⇒ 自证把「夹具没隔离」误报成「豁免表没生效」（实测踩过）。
    rm -f "$T/tools/x.sh"
    printf '%s\n' 'echo ok' > "$T/examples/fake/verify.sh"
    printf '%s\n' 'P = "/data/code/puxian/runtime/runtime.c"' > "$T/examples/fake/negctl.py"
    bad4="$(scan "$T" | wc -l)"
    # ⑤ 豁免表生效：登记该文件后 ⇒ 不再报
    #    ⚠️ 首版把路径写进了**注释行**（`printf '%s\texamples/...' '# selftest'`）⇒ 第一列是 `# selftest`
    #    ⇒ 表里根本没有这条路径 ⇒ 自证假红。夹具要**分成两行**写。
    printf '%s\n' '# selftest' 'examples/fake/negctl.py	selftest' > "$T/allow.tsv"
    bad5="$(scan "$T" "$T/allow.tsv" | wc -l)"
    rm -rf "$T"
    ok=0; fail=0
    [ "$bad1" -ge 1 ] && { echo "  ✅ ① 代码行写死开发机路径 ⇒ 抓到 $bad1 条"; ok=$((ok+1)); } \
                      || { echo "  ❌ ① 未抓到（判据无牙）"; fail=$((fail+1)); }
    [ "$bad2" -eq 0 ] && { echo "  ✅ ② 注释行记载反模式 ⇒ 不抓（允许留证）"; ok=$((ok+1)); } \
                      || { echo "  ❌ ② 把注释也抓了（误报）"; fail=$((fail+1)); }
    [ "$bad3" -ge 1 ] && { echo "  ✅ ③ 写死 CI 工作区前缀 ⇒ 抓到 $bad3 条"; ok=$((ok+1)); } \
                      || { echo "  ❌ ③ 未抓到"; fail=$((fail+1)); }
    [ "$bad4" -ge 1 ] && { echo "  ✅ ④ .py 辅助脚本写死 ⇒ 抓到 $bad4 条（M317 扩面）"; ok=$((ok+1)); } \
                      || { echo "  ❌ ④ .py 里写死没抓到（扩面失效）"; fail=$((fail+1)); }
    [ "$bad5" -eq 0 ] && { echo "  ✅ ⑤ 豁免表生效（登记后不再报）"; ok=$((ok+1)); } \
                      || { echo "  ❌ ⑤ 豁免表没生效"; fail=$((fail+1)); }
    echo "check_gate_paths self-test: $ok 通过 / $fail 失败"
    [ "$fail" -eq 0 ] || exit 2
fi

VIOL="$(scan "$ROOT" "$ALLOW")"
# 全量违例（**不带豁免**）—— 只算一次：过期判据要问「这个文件本身还有没有违例」，
# ⚠️ 曾在循环里对每个豁免行重扫全仓（18 × 全仓）⇒ 30s 超时；改成先算一次缓存。
RAW="$(scan "$ROOT")"
# 豁免表**过期**判据：登记的文件若已无违例 ⇒ 判红（逼删行；表的目标是变空）
STALE=""
if [ -f "$ALLOW" ]; then
    while IFS=$'\t' read -r rel why; do
        case "$rel" in ''|'#'*) continue ;; esac
        if [ -z "${why:-}" ]; then
            STALE="${STALE}${rel}	（理由为空 —— 无理由豁免 = 把红当绿）
"
            continue
        fi
        [ -f "$ROOT/$rel" ] || { STALE="${STALE}${rel}	（文件不存在）
"; continue; }
        # 该文件若**不在**全量违例清单里（未被豁免时本会报）⇒ 说明已修好 ⇒ 删行
        if ! grep -qF "/$rel:" <<<"$RAW"; then
            STALE="${STALE}${rel}	（已无违例 ⇒ 请删除本行）
"
        fi
    done < "$ALLOW"
fi

RC=0
if [ -n "$VIOL" ]; then
    echo "❌ 门里出现**开发机绝对路径**（本机绿 / CI 红 —— 缺陷 300；分片档会污染被测树 —— M317）："
    echo "$VIOL" | sed 's/^/     /'
    echo "   ⇒ 改用按脚本位置推导：HERE=\$(cd \"\$(dirname \"\$0\")\" && pwd); ROOT=\$(cd \"\$HERE/../..\" && pwd)"
    RC=1
fi
if [ -n "$STALE" ]; then
    echo "❌ 豁免表过期/无理由（gate_paths_allow.tsv · 只减不增）："
    echo "$STALE" | sed 's/^/     /'
    RC=1
fi
[ "$RC" = 0 ] || exit 1
NA="$(cut -f1 "$ALLOW" 2>/dev/null | grep -vcE '^#|^$' || true)"
echo "✅ 门路径卫生：$(gate_files "$ROOT" | wc -l) 个门/脚本（含 .py 与子目录 .sh）+ workflows —— 无开发机绝对路径（豁免存量 ${NA:-0} 个，均带理由）"
exit 0
