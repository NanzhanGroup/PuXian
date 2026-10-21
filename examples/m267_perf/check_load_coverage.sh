#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────
# check_load_coverage.sh —— ONNX 负载清单 ⇄ 算子注册表 **双向核对**（M310 · 丙2 W1/W5）
#
# 为什么需要它（W0-① 实测的缺口）：
#   `verify.sh` 的测量面 = **选定档的行**（`read_baseline()`），而档里出现未知 ONNX 名
#   只会判「无法测量」—— **不会**提醒「某个已注册算子从没被任何负载覆盖」。
#   ⇒ 注册表新增算子、负载表忘了跟 ⇒ 账本里静默少一项，**没有任何门看得见**。
#   本脚本把这条缺口变成一条**源码派生**的判据：注册表 ⇄ 清单，缺一个即判红。
#
# 判据（三向，各自独立判红，不互相代偿）：
#   ① 范围族（源码派生：`runtime/onnx_ops.c` 的 `g_ops[]` 中注释标 M306/M308 的条目）
#      \ 清单覆盖集 ≠ ∅ ⇒ 判红「已接进 runtime、但性能账本看不见」
#   ② 清单覆盖集 \ 注册表全量 ≠ ∅ ⇒ 判红「清单写了不存在的算子（拼错 / 已删）」
#   ③ 清单的**可执行性**（防「清单有、门不跑」）：
#        · 每个负载必须有 `src/<load>.px`（驱动源码）
#        · 模型文件名必须出现在生成器 `tools/gen_perf_models.py`（否则模型不产出）
#        · `verify.sh` 必须**从清单派生** ONNX_LOADS（不得回退成硬编码 —— 那是 W0-① 的病）
#   ④ 清单 ⇄ **每个基线档**：每个负载必须在**每一份**档里有一行 —— 缺行 ⇒ 该负载
#      永不进测量面（W0-①）；且合成档（CI）= 各档逐负载 max ⇒ 任一档缺行都会让它消失。
#
# 清单格式（`examples/m267_perf/onnx_loads.tsv`）：
#   <负载名> <模型文件> <覆盖算子（逗号分隔）> <说明>
#
# 用法：
#   bash examples/m267_perf/check_load_coverage.sh             # 正式核对（rc=0 绿）
#   bash examples/m267_perf/check_load_coverage.sh --selftest   # 自证 3 例（全在/缺一/多一）
#   环境：M267_COVERAGE_REPO（源码树）· M267_COVERAGE_ROOT（夹具根）·
#         M267_COVERAGE_TABLE（清单）· M267_COVERAGE_VERIFY（门实现）
# 环境：M267_COVERAGE_BASELINES（档目录）
# 退出码：0 绿 · 1 红 · 2 环境/用法错误
# ─────────────────────────────────────────────────────────────────────
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="${M267_COVERAGE_REPO:-$(cd "$HERE/../.." && pwd)}"
ROOT="${M267_COVERAGE_ROOT:-$REPO}"
OPS_C="$REPO/runtime/onnx_ops.c"
LOAD_TABLE="${M267_COVERAGE_TABLE:-$ROOT/examples/m267_perf/onnx_loads.tsv}"
VERIFY="${M267_COVERAGE_VERIFY:-$ROOT/examples/m267_perf/verify.sh}"
GEN="${M267_COVERAGE_GEN:-$ROOT/examples/m267_perf/tools/gen_perf_models.py}"
SRC="$ROOT/examples/m267_perf/src"
BLDIR="${M267_COVERAGE_BASELINES:-$ROOT/examples/m267_perf/baselines}"

die() { echo "❌ $*" >&2; exit 2; }
red() { echo "  ❌ $*"; RC=1; }
ok()  { echo "  ✅ $*"; }

# ── 源码派生：g_ops[] 的 (算子名, 所属注释) ──────────────────────────
# 注释块后**连续**的条目共享该注释（M306/M308 的块注释覆盖多行条目）。
derive_ops() { # $1=file
    awk '
      /static const OnnxOpEntry g_ops\[\] *= *\{/ { inside=1; next }
      inside && /^};/ { inside=0 }
      !inside { next }
      {
        line=$0
        if (line ~ /^[ \t]*\/\*/) { pend=line; next }
        if (line ~ /^[ \t]*\*/)   { pend=pend " " line; next }
        # 一行可能有多条 { "Name", fn }
        while (match(line, /\{[ \t]*"[A-Za-z0-9_]+"/)) {
            s=substr(line, RSTART, RLENGTH)
            gsub(/[{ \t"]/, "", s)
            print s "\t" pend
            line=substr(line, RSTART+RLENGTH)
        }
      }
    ' "$1"
}

# ── 清单：<负载名> <模型文件> <算子,算子...> <说明> ────────────────
read_table() { grep -v '^[[:space:]]*#' "$1" 2>/dev/null | grep -v '^[[:space:]]*$'; }

check() { # $1=ops_c $2=load_table $3=verify $4=gen $5=src $6=baselines
    RC=0
    [ -f "$1" ] || die "缺算子注册表：$1"
    [ -f "$2" ] || die "缺负载清单：$2"
    [ -f "$3" ] || die "缺门实现：$3"
    [ -f "$4" ] || die "缺模型生成器：$4"

    local ops_all ops_fleet covered loads
    ops_all=$(derive_ops "$1" | cut -f1 | sort -u)
    ops_fleet=$(derive_ops "$1" | awk -F'\t' '$2 ~ /M306|M308/ {print $1}' | sort -u)
    covered=$(read_table "$2" | awk '{n=split($3,a,","); for(i=1;i<=n;i++) if(a[i]!="") print a[i]}' | sort -u)
    loads=$(read_table "$2" | awk '{print $1}' | sort -u)

    echo "══ ONNX 负载清单 ⇄ 算子注册表（M310 · 丙2 W1/W5）══"
    echo "  注册表算子：$(printf '%s\n' "$ops_all" | wc -l) 个 · M306/M308 范围族：$(printf '%s\n' "$ops_fleet" | wc -l) 个 · 清单覆盖：$(printf '%s\n' "$covered" | wc -l) 个 · 负载：$(printf '%s\n' "$loads" | wc -l) 个"

    local miss extra
    miss=$(comm -23 <(printf '%s\n' "$ops_fleet") <(printf '%s\n' "$covered"))
    extra=$(comm -13 <(printf '%s\n' "$ops_all") <(printf '%s\n' "$covered"))
    if [ -n "$miss" ]; then
        red "范围族算子未被任何负载覆盖（$(printf '%s\n' "$miss" | wc -l) 个）：$(printf '%s' "$miss" | tr '\n' ' ')"
    else
        ok "范围族算子全部被负载覆盖（$(printf '%s\n' "$ops_fleet" | wc -l)/$(printf '%s\n' "$ops_fleet" | wc -l)）"
    fi
    if [ -n "$extra" ]; then
        red "清单覆盖集含注册表里不存在的算子（$(printf '%s\n' "$extra" | wc -l) 个）：$(printf '%s' "$extra" | tr '\n' ' ')"
    else
        ok "清单覆盖集全部可解析（无拼错 / 无已删算子）"
    fi

    # ③ 可执行性：驱动源码 / 模型条目 / 门是否从清单派生（不得回退硬编码）
    local badsrc badgen
    badsrc=""; badgen=""
    while read -r l mdl _rest; do
        [ -n "$l" ] || continue
        [ -f "$5/$l.px" ] || badsrc="$badsrc $l"
        grep -q "\"$mdl\"" "$4" || badgen="$badgen $mdl"
    done < <(read_table "$2")
    if [ -n "$badsrc" ]; then red "清单负载缺驱动源码 src/<load>.px：$badsrc"; else ok "清单负载全部有驱动源码（src/*.px）"; fi
    if [ -n "$badgen" ]; then red "清单模型未在生成器登记（不会产出）：$badgen"; else ok "清单模型全部在 tools/gen_perf_models.py 中登记"; fi
    if grep -q 'ONNX_MANIFEST' "$3"; then
        ok "门从清单派生 ONNX_LOADS（单一事实源，非硬编码）"
    else
        red "门实现未从清单派生（ONNX_MANIFEST 缺失）⇒ 清单会与派发脱钩（W0-① 的病）"
    fi

    # ④ 每个档都要有每个负载的行（缺行 = 永不测量）
    local ntier=0 missb=""
    if [ -d "$6" ]; then
        for tf in "$6"/*.tsv; do
            [ -f "$tf" ] || continue
            ntier=$((ntier+1))
            while read -r l _rest; do
                [ -n "$l" ] || continue
                grep -qE "^$l[[:space:]]" "$tf" || missb="$missb $(basename "$tf"):$l"
            done < <(read_table "$2")
        done
    fi
    if [ "$ntier" -eq 0 ]; then
        red "找不到基线档（$6）⇒ 无法确认负载会进测量面"
    elif [ -n "$missb" ]; then
        red "以下「档:负载」未登记 ⇒ 该负载在该档**永不测量**：$missb"
    else
        ok "清单负载在全部 $ntier 个档里均有登记（测量面覆盖）"
    fi
    return $RC
}

# ── 自证 3 例：全在 / 缺一 / 多一 ───────────────────────────────────
selftest() {
    local fx rc fail=0
    fx=$(mktemp -d /tmp/m267_w1_selftest.XXXXXX)
    mkdir -p "$fx/runtime" "$fx/examples/m267_perf/src" "$fx/examples/m267_perf/tools"
    cp "$OPS_C" "$fx/runtime/onnx_ops.c"
    cp "$VERIFY" "$fx/examples/m267_perf/verify.sh"
    cp "$GEN" "$fx/examples/m267_perf/tools/gen_perf_models.py"

    local fleet
    fleet=$(derive_ops "$OPS_C" | awk -F'\t' '$2 ~ /M306|M308/ {print $1}' | sort -u | tr '\n' ',' | sed 's/,$//')
    touch "$fx/examples/m267_perf/src/onnx_fixture.px"
    mkdir -p "$fx/examples/m267_perf/baselines"
    printf 'onnx_fixture   onnx   s   0.2   夹具\n' > "$fx/examples/m267_perf/baselines/fixture.tsv"

    t() { # $1=算子串 $2=模型名
        printf '# 夹具\nonnx_fixture %s %s 夹具\n' "$2" "$1" > "$fx/examples/m267_perf/onnx_loads.tsv"
        # ⚠️ M317 修（缺陷 300 同族）：原先写死 `M267_COVERAGE_REPO=/data/code/puxian`
        #   ⇒ 分片 worktree 里跑自证时扫的是**主仓库**（与 worktree 同提交才等价）。
        #   脚本头已有 `REPO="${M267_COVERAGE_REPO:-$(cd "$HERE/../.." && pwd)}"` ⇒ 用 "$REPO" 即跟随被测树。
        M267_COVERAGE_REPO="$REPO" M267_COVERAGE_ROOT="$fx" \
          M267_COVERAGE_TABLE="$fx/examples/m267_perf/onnx_loads.tsv" \
          M267_COVERAGE_VERIFY="$fx/examples/m267_perf/verify.sh" \
          M267_COVERAGE_GEN="$fx/examples/m267_perf/tools/gen_perf_models.py" \
          M267_COVERAGE_BASELINES="$fx/examples/m267_perf/baselines" \
          bash "$0" >>/tmp/m267_w1_case.log 2>&1
    }

    echo "── 例① 全在 ⇒ 必须绿"
    # 模型名要出现在生成器里：用一个真实存在的模型名占位
    : >/tmp/m267_w1_case.log
    t "$fleet" "onnx_mlp.onnx"
    rc=$?
    if [ "$rc" = 0 ]; then echo "  ✅ 例① 绿（rc=0）"; else echo "  ❌ 例① 应为绿，实测 rc=$rc（见 /tmp/m267_w1_case.log）"; fail=1; fi

    echo "── 例② 缺一 ⇒ 必须红"
    t "$(printf '%s' "$fleet" | sed 's/,[^,]*$//')" "onnx_mlp.onnx"
    rc=$?
    if [ "$rc" = 1 ]; then echo "  ✅ 例② 红（rc=1）"; else echo "  ❌ 例② 应为红，实测 rc=$rc"; fail=1; fi

    echo "── 例③ 多一 ⇒ 必须红"
    t "$fleet,NoSuchOp" "onnx_mlp.onnx"
    rc=$?
    if [ "$rc" = 1 ]; then echo "  ✅ 例③ 红（rc=1）"; else echo "  ❌ 例③ 应为红，实测 rc=$rc"; fail=1; fi

    echo "══ W1 清单脚本自证：$([ $fail = 0 ] && echo '3/3 ✅' || echo '有未生效 ❌') ══"
    echo "   夹具：$fx · 日志 /tmp/m267_w1_case.log"
    return $fail
}

case "${1:-}" in
  --selftest) selftest; exit $? ;;
  "") check "$OPS_C" "$LOAD_TABLE" "$VERIFY" "$GEN" "$SRC" "$BLDIR"; exit $? ;;
  *) echo "用法：$0 [--selftest]" >&2; exit 2 ;;
esac
