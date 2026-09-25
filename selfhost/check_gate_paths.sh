#!/usr/bin/env bash
# ============================================================
# selfhost/check_gate_paths.sh —— **门里不许出现开发机绝对路径**（M213 · 缺陷 300）
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
# 判据：扫 `examples/*/verify.sh` · `selfhost/*.sh` · `tools/*.sh` · `.github/workflows/*.yml`
#   逐行找 `/data/code/` 与写死的 CI 工作区前缀 `/home/runner/work/`：
#     · **代码行**（首个非空白字符不是 `#`）⇒ **违例**（判红）
#     · **注释行** ⇒ 允许（m156/m157 就用注释**记载这个反模式**，是有价值的留证）
# 自带自证（2 正 + 2 负），失败即红。
# 用法：bash selfhost/check_gate_paths.sh [--self-test]
# 退出码：0 无违例 / 1 有违例 / 2 自证失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"

PATTERNS=('/data/code/' '/home/runner/work/')

scan() {   # $1 = 仓库根；输出违例行 file:lineno:content
    local root="$1" f line no n=0
    for f in "$root"/examples/*/verify.sh "$root"/selfhost/*.sh "$root"/tools/*.sh \
             "$root"/.github/workflows/*.yml; do
        [ -f "$f" ] || continue
        # ⚠️ **跳过本脚本自己**：它的 PATTERNS 与自证 fixture 里**按定义**就含这两个前缀
        #   ⇒ 不跳过的话，判据会**把自己判红**（首版实测：5 条"违例"全是它自己）。
        case "$(basename "$f")" in check_gate_paths.sh) continue ;; esac
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
    done
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
    rm -rf "$T"
    ok=0; fail=0
    [ "$bad1" -ge 1 ] && { echo "  ✅ ① 代码行写死开发机路径 ⇒ 抓到 $bad1 条"; ok=$((ok+1)); } \
                      || { echo "  ❌ ① 未抓到（判据无牙）"; fail=$((fail+1)); }
    [ "$bad2" -eq 0 ] && { echo "  ✅ ② 注释行记载反模式 ⇒ 不抓（允许留证）"; ok=$((ok+1)); } \
                      || { echo "  ❌ ② 把注释也抓了（误报）"; fail=$((fail+1)); }
    [ "$bad3" -ge 1 ] && { echo "  ✅ ③ 写死 CI 工作区前缀 ⇒ 抓到 $bad3 条"; ok=$((ok+1)); } \
                      || { echo "  ❌ ③ 未抓到"; fail=$((fail+1)); }
    echo "check_gate_paths self-test: $ok 通过 / $fail 失败"
    [ "$fail" -eq 0 ] || exit 2
fi

VIOL="$(scan "$ROOT")"
if [ -n "$VIOL" ]; then
    echo "❌ 门里出现**开发机绝对路径**（本机绿 / CI 红 —— 缺陷 300）："
    echo "$VIOL" | sed 's/^/     /'
    echo "   ⇒ 改用按脚本位置推导：HERE=\$(cd \"\$(dirname \"\$0\")\" && pwd); ROOT=\$(cd \"\$HERE/../..\" && pwd)"
    exit 1
fi
echo "✅ 门路径卫生：$(ls "$ROOT"/examples/*/verify.sh | wc -l) 个门 + selfhost/tools 脚本 + workflows —— 无开发机绝对路径"
exit 0
