#!/usr/bin/env bash
# ============================================================
# run_upstream_tests.sh —— 上游 registry-px「真实用例」回归（M190 建立）
# ------------------------------------------------------------
# 为什么有这道门：
#   M187/M189 把上游 53 库引入 registry/ 后，只做了 **import 冒烟**（能编译、能调到入口）。
#   冒烟证明不了「库能用」—— 上游 tests/*.px 才是逐库的行为断言（2959 行、53 个用例）。
#   本脚本把它们跑在**我方官方 registry** 上，闭合「照搬 ⇒ 可用」这一环。
#
# 关键设计（三条判据来自此前事故，别改）：
#   ① 用例**逐字节照搬** ⇒ 期望值写在数据文件 EXPECTED.tsv 里（不写成"实际跑出来是什么"，
#      否则门会自我漂绿）；MANIFEST.sha256 逐文件对拍，本地改动即硬失败。
#   ② **每个引用的模块必须存在**（`--check-imports`）—— M188 事故：stdlib 定位失败时
#      "找不到模块"只是**警告**、程序照跑 ⇒ 用例静默降级。这里把"引用面完整"变成前置判据。
#   ③ fixture 由本脚本自建（上游仓库**没有** fixture 脚本，dotenv/glob 用例依赖 /tmp 预置）。
#
# 目录契约：用例里的相对 import 是 `../registry/<name>/<ver>/<name>.px`，
#   因此把用例放在 <repo>/upstream-tests/ 下 ⇒ 正好命中 <repo>/registry/（我方官方包）。
#
# 用法：
#   selfhost/run_upstream_tests.sh [--track build|interp|both] [--only a,b,c] [--list]
#                                  [--registry <dir>] [--work <dir>] [--keep]
#                                  [--no-fixtures] [--run-skipped] [--timeout <sec>]
#                                  [--expected <file>] [--json <file>] [-v]
# 退出码：0 = 与 EXPECTED 完全一致；非 0 = 有偏离（未预期失败 / 未预期跳过 / 清单漂移）
# ============================================================
set -u

ROOT="$(cd "$(dirname "$(readlink -f "$0" 2>/dev/null || echo "$0")")/.." && pwd)"
TESTS_DIR="$ROOT/upstream-tests"
REGISTRY="$ROOT/registry"
EXPECTED="$TESTS_DIR/EXPECTED.tsv"
MANIFEST="$TESTS_DIR/MANIFEST.sha256"
PX="$ROOT/tools/px"
TRACK=both
ONLY=""
WORK=""
KEEP=0
FIXTURES=1
RUN_SKIPPED=0
TIMEOUT=180
JSON=""
VERBOSE=0
LIST=0

while [ $# -gt 0 ]; do
    case "$1" in
        --track) TRACK="$2"; shift 2 ;;
        --only) ONLY="$2"; shift 2 ;;
        --registry) REGISTRY="$2"; shift 2 ;;
        --work) WORK="$2"; shift 2 ;;
        --expected) EXPECTED="$2"; shift 2 ;;
        --json) JSON="$2"; shift 2 ;;
        --timeout) TIMEOUT="$2"; shift 2 ;;
        --keep) KEEP=1; shift ;;
        --no-fixtures) FIXTURES=0; shift ;;
        --run-skipped) RUN_SKIPPED=1; shift ;;
        --list) LIST=1; shift ;;
        -v) VERBOSE=1; shift ;;
        -h|--help) sed -n '2,30p' "$0"; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done

case "$TRACK" in build|interp|both) ;; *) echo "--track 只能 build|interp|both" >&2; exit 2 ;; esac
[ -x "$PX" ] || { echo "找不到可执行工具链: $PX" >&2; exit 2; }
[ -d "$TESTS_DIR" ] || { echo "找不到用例目录: $TESTS_DIR" >&2; exit 2; }
[ -d "$REGISTRY" ] || { echo "找不到 registry: $REGISTRY" >&2; exit 2; }
[ -f "$EXPECTED" ] || { echo "找不到期望值文件: $EXPECTED" >&2; exit 2; }

# ---------- 前置判据 ①：用例逐字节照搬（MANIFEST 对拍） ----------
DRIFT=0
if [ -f "$MANIFEST" ]; then
    if ! ( cd "$TESTS_DIR" && sha256sum -c MANIFEST.sha256 ) >/tmp/.ut_manifest.$$ 2>&1; then
        echo "✗ 用例清单漂移（upstream-tests/ 下有文件被改动/缺失）："
        grep -v ': OK$' /tmp/.ut_manifest.$$ | sed 's/^/    /'
        DRIFT=1
    fi
    rm -f /tmp/.ut_manifest.$$
else
    echo "✗ 缺少 MANIFEST.sha256（无法证明用例逐字节照搬）"; DRIFT=1
fi
[ "$DRIFT" = 1 ] && exit 1

# ---------- 期望值（数据文件） ----------# 行格式： <测试名>\t<build 期望>\t<interp 期望>\t<理由>
# 取值 PASS / SKIP；# 开头为注释。
exp_for() {  # $1=test  $2=track  → 打印 PASS|SKIP|MISSING
    awk -F'\t' -v t="$1" -v k="$2" '
        /^[[:space:]]*#/ || NF==0 { next }
        $1==t { if (k=="build") print $2; else print $3; found=1 }
        END { if (!found) print "MISSING" }' "$EXPECTED"
}
reason_for() {
    awk -F'\t' -v t="$1" '/^[[:space:]]*#/ || NF==0 { next } $1==t { print (NF>=4?$4:"-") }' "$EXPECTED"
}

ALL=()
while IFS= read -r f; do ALL+=("${f%.px}"); done < <(cd "$TESTS_DIR" && ls *_test.px | sort)
if [ "$LIST" = 1 ]; then
    printf '%-24s %-6s %-6s %s\n' 测试 build interp 理由
    for t in "${ALL[@]}"; do
        printf '%-24s %-6s %-6s %s\n' "$t" "$(exp_for "$t" build)" "$(exp_for "$t" interp)" "$(reason_for "$t")"
    done
    exit 0
fi

SEL=()
if [ -n "$ONLY" ]; then
    IFS=',' read -r -a want <<< "$ONLY"
    for t in "${ALL[@]}"; do
        for w in "${want[@]}"; do [ "$t" = "$w" ] && SEL+=("$t"); done
    done
    [ "${#SEL[@]}" -eq 0 ] && { echo "--only 未匹配任何用例" >&2; exit 2; }
else
    SEL=("${ALL[@]}")
fi

# ---------- 前置判据 ②：引用面完整（每个 ../registry/... 必须真实存在） ----------
echo "══ 上游 registry-px 真实用例回归（M190） ══"
echo "用例目录: $TESTS_DIR（${#SEL[@]}/${#ALL[@]} 个）"
echo "registry : $REGISTRY"
echo "工具链   : $($PX --version 2>&1 | head -1)"
echo "── 引用面检查 ──"
MISSING_REF=0
for t in "${SEL[@]}"; do
    while IFS= read -r rel; do
        [ -z "$rel" ] && continue
        # 用例里的相对 import 是相对**用例文件**的 `../registry/...`
        # ⇒ 判据要落在**被测 registry** 上（--registry 可覆盖，负控用）
        p="$REGISTRY/${rel#../registry/}"
        if [ ! -f "$p" ]; then
            echo "  ✗ $t → 引用缺失: $rel"
            MISSING_REF=$((MISSING_REF + 1))
        fi
    done < <(grep -o '\.\./registry/[^"]*' "$TESTS_DIR/$t.px" | sort -u)
done
if [ "$MISSING_REF" != 0 ]; then
    echo "✗ 有 $MISSING_REF 处引用在 registry 中不存在（用例会静默降级，M188 事故族）"
    exit 1
fi
echo "  ✓ 全部引用存在"

# ---------- 工作区（staged 树：让 ../registry 指向被测 registry） ----------
CLEANUP=1
if [ -z "$WORK" ]; then
    WORK="$(mktemp -d /tmp/px-upstream-tests.XXXXXX)"
else
    mkdir -p "$WORK"; CLEANUP=0
fi
[ "$KEEP" = 1 ] && CLEANUP=0
cleanup() { [ "$CLEANUP" = 1 ] && rm -rf "$WORK"; return 0; }
trap cleanup EXIT
mkdir -p "$WORK/tests"
cp -p "$TESTS_DIR"/*.px "$WORK/tests/" 2>/dev/null || true
ln -sfn "$(readlink -f "$REGISTRY")" "$WORK/registry"

# ---------- fixture（上游仓库无 fixture 脚本 ⇒ 本脚本自建） ----------
if [ "$FIXTURES" = 1 ]; then
    printf 'USER=test\nQUOTED="a b"\n' > /tmp/dotenv_sample.env
    [ "/tmp/globd" = "/tmp/globd" ] && rm -rf /tmp/globd
    mkdir -p /tmp/globd/sub
    : > /tmp/globd/a.log
    : > /tmp/globd/b.txt
    : > /tmp/globd/sub/c.log
    # M200：walk 用例的 fixture（上游 `walk_test.px` 断言的**正是这 7 项**：
    #   root + a.txt + b.log + sub + sub/c.txt + sub/deep + sub/deep/d.txt）
    rm -rf /tmp/wk_src
    mkdir -p /tmp/wk_src/sub/deep
    : > /tmp/wk_src/a.txt
    : > /tmp/wk_src/b.log
    : > /tmp/wk_src/sub/c.txt
    : > /tmp/wk_src/sub/deep/d.txt
    rm -rf /tmp/shutil_registry_test
fi
: > "${JSON:-/dev/null}"

# ---------- 跑 ----------
PASS=0; FAIL=0; SKIP=0; MISMATCH=0
: > /tmp/.ut_res.$$
run_one() {  # $1=track  $2=test  → 设 RC / OUT
    local trk="$1" t="$2" log="$WORK/$1-$2.log"
    if [ "$trk" = build ]; then
        ( cd "$WORK/tests" && timeout "$TIMEOUT" "$PX" build "$t.px" ) >"$log.b" 2>&1
        local brc=$?
        if [ $brc -ne 0 ]; then RC=$brc; OUT="$(tail -3 "$log.b")"; return; fi
        ( cd "$WORK/tests" && timeout "$TIMEOUT" "./build/$t" ) >"$log" 2>&1
        RC=$?
    else
        ( cd "$WORK/tests" && timeout "$TIMEOUT" "$PX" run "$t.px" ) >"$log" 2>&1
        RC=$?
    fi
    OUT="$(cat "$log")"
}

for t in "${SEL[@]}"; do
    for trk in build interp; do
        [ "$TRACK" != both ] && [ "$TRACK" != "$trk" ] && continue
        ex="$(exp_for "$t" "$trk")"
        if [ "$ex" = MISSING ]; then
            echo "  ✗ $t[$trk] 不在 EXPECTED.tsv 里（新增用例必须显式登记期望值）"
            MISMATCH=$((MISMATCH + 1)); printf '%s\t%s\tMISSING\t-\n' "$t" "$trk" >> /tmp/.ut_res.$$
            continue
        fi
        if [ "$ex" = SKIP ] && [ "$RUN_SKIPPED" = 0 ]; then
            SKIP=$((SKIP + 1))
            printf '%s\t%s\tSKIP\t%s\n' "$t" "$trk" "$(reason_for "$t")" >> /tmp/.ut_res.$$
            continue
        fi
        run_one "$trk" "$t"
        if [ "$RC" = 0 ] && printf '%s' "$OUT" | grep -q 'PASS'; then
            got=PASS
        elif [ "$RC" = 124 ]; then
            got=TIMEOUT
        else
            got=FAIL
        fi
        if [ "$ex" = SKIP ]; then   # --run-skipped 下跑了期望跳过的：只作信息
            [ "$VERBOSE" = 1 ] && echo "  ℹ $t[$trk] 期望 SKIP，实跑 $got"
        elif [ "$got" = "$ex" ]; then
            PASS=$((PASS + 1))
            [ "$VERBOSE" = 1 ] && echo "  ✓ $t[$trk]"
        else
            echo "  ✗ $t[$trk] 期望 $ex 实际 $got（rc=$RC）"
            printf '%s\n' "$OUT" | sed -n '1,4p' | sed 's/^/      | /'
            FAIL=$((FAIL + 1))
        fi
        printf '%s\t%s\t%s\t%d\n' "$t" "$trk" "$got" "$RC" >> /tmp/.ut_res.$$
    done
done

# ---------- 汇总 ----------
echo "── 结果 ──"
awk -F'\t' '{c[$3]++} END {for (k in c) printf "  %-8s %d\n", k, c[k]}' /tmp/.ut_res.$$ | sort
echo "  通过 $PASS · 失败 $FAIL · 跳过 $SKIP · 期望值缺失 $MISMATCH"
if [ -n "$JSON" ]; then
    # ⚠️ SKIP 行的第 4 列是**理由**（不是 rc）⇒ 生成 JSON 时必须区分，
    #   否则产出 `"rc":interp 设计性不支持 …`（非法 JSON —— 首版就踩了这个）。
    { echo "{"; echo '  "tests": [';
      awk -F'\t' 'BEGIN{n=0} {n++; rc=($4 ~ /^-?[0-9]+$/ ? $4 : "null");
        printf "%s    {\"test\":\"%s\",\"track\":\"%s\",\"result\":\"%s\",\"rc\":%s}", (n>1?",\n":""), $1,$2,$3,rc} END{print ""}' /tmp/.ut_res.$$;
      echo "  ],"; echo "  \"pass\": $PASS, \"fail\": $FAIL, \"skip\": $SKIP, \"mismatch\": $MISMATCH"; echo "}"; } > "$JSON"
fi
rm -f /tmp/.ut_res.$$
if [ "$FAIL" != 0 ] || [ "$MISMATCH" != 0 ]; then
    echo "══ 汇总：上游用例回归 与 EXPECTED 不一致 ══"
    [ "$KEEP" = 1 ] && echo "工作区保留: $WORK"
    exit 1
fi
echo "══ 汇总：上游用例回归 全部符合 EXPECTED ══"
[ "$KEEP" = 1 ] && echo "工作区保留: $WORK"
exit 0
