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
#      ⚠️ M282：期望值有三个等级 —— **PASS**（必须通过）· **SKIP**（本环境不跑，理由必填）·
#         **XFAIL**（**期望失败**：上游 0.2.0 的用例与它**自己的库**自相矛盾，见
#         docs/UPSTREAM_020_DEFECTS.md）。XFAIL 带**反向判据** —— 一旦它通过了就**判红**
#         （等于上游修好了，必须回来改登记），所以它**不是**"把红记成绿"的出口。
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
# 退出码：0 = 与 EXPECTED 完全一致；非 0 = 有偏离（未预期失败 / XFAIL 意外通过 / 清单漂移）
#         **抖动（首跑失败、确认步通过）不判非零**，但响亮计数；≥3 次视为系统性问题 ⇒ 非零。
#   --no-confirm：关掉确认步（仅用于定性排查，不用于验收）
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
        --no-confirm) CONFIRM=0; shift ;;
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

# ---------- 前置判据 ①b：MANIFEST 与用例目录**双向一致**（M282 新增） ----------
#   为什么加这一条：`sha256sum -c` 只验「清单里**已列**的文件内容对不对」，
#   看不见「清单**漏列**了用例」或「清单里混进了**本仓自己的**文件」。M282 实测事故：
#   重建清单时写成 `sha256sum *.px fixtures/*.px` ⇒ 我们一改 mock（**本仓文件**，
#   按 fixtures/README.md 本就**不该**进清单）就把「用例逐字节照搬」判据弄红 ——
#   判据指不到真因。⇒ 判据：清单的条目集 **必须恰好等于** `*_test.px` 集（多一个/少一个都判红）。
MF_SET="$(awk 'NF>=2 {n=$2; sub(/^\*/,"",n); print n}' "$MANIFEST" | sort)"
FS_SET="$(cd "$TESTS_DIR" && ls *_test.px | sort)"
if [ "$MF_SET" != "$FS_SET" ]; then
    echo "✗ MANIFEST.sha256 与用例目录**双向不一致**（清单条目集 ≠ *_test.px 集）："
    diff <(printf '%s\n' "$MF_SET") <(printf '%s\n' "$FS_SET") | sed 's/^/    /'
    exit 1
fi

# ---------- 前置判据 ①c：EXPECTED 与用例目录**双向一致 + 无重复**（M282 新增） ----------
#   为什么加：`exp_for` 的 awk 取「**最后**一个匹配」⇒ 若同一用例被登记两次且取值不同，
#   前一条会被**静默忽略**（判据静默变窄，M199/M227 同族）。
#   M282 实测：生成器用 `*2_test.px` 通配 ⇒ 把 0.1.0 的 `oauth2_test.px`（名字以 "2" 结尾）
#   也当成新用例重复登记了一次；另有两个 0.2.0 用例在更早的轮次已登记。⇒ 三条重复全部来自
#   「只查单向、不查重复」。这里把它变成**前置硬判据**。
EXP_NAMES="$(awk -F'\t' '/^[[:space:]]*#/ || NF==0 {next} {print $1}' "$EXPECTED" | sort)"
DUP="$(printf '%s\n' "$EXP_NAMES" | uniq -d)"
if [ -n "$DUP" ]; then
    echo "✗ EXPECTED.tsv 有**重复登记**（后一条会静默覆盖前一条）："
    printf '%s\n' "$DUP" | sed 's/^/    /'
    exit 1
fi
EXP_FILES="$(cd "$TESTS_DIR" && ls *_test.px | sed 's/\.px$//' | sort)"
if [ "$EXP_NAMES" != "$EXP_FILES" ]; then
    echo "✗ EXPECTED.tsv 与用例目录**双向不一致**："
    diff <(printf '%s\n' "$EXP_NAMES") <(printf '%s\n' "$EXP_FILES") | sed 's/^/    /'
    exit 1
fi

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
NET_PIDS=()
cleanup() {
    for p in "${NET_PIDS[@]:-}"; do
        [ -n "$p" ] || continue
        kill "$p" 2>/dev/null
        # ⚠️ **不要**写 `kill -- "-$p"`：脚本里后台作业与脚本**同进程组** ⇒ 那是**杀自己**。
        sleep 0.2
        kill -9 "$p" 2>/dev/null
    done
    [ "$CLEANUP" = 1 ] && rm -rf "$WORK"
    return 0
}
trap cleanup EXIT
mkdir -p "$WORK/tests"
cp -p "$TESTS_DIR"/*.px "$WORK/tests/" 2>/dev/null || true
ln -sfn "$(readlink -f "$REGISTRY")" "$WORK/registry"

# ---------- fixture（上游仓库无 fixture 脚本 ⇒ 本脚本自建） ----------
# M282 增补：**环境变量 fixture** —— 上游 0.2.0 的 cli2/config2 用例断言「env 优先于 default」：
#   cli2     spec4.host.env = "PX_TEST_HOST"      ⇒ 期望 **env-host**
#   config2  cfg_apply_env_recursive(cfg, "APP")  ⇒ 期望 **env-db-host**
#   且 cli2 另有一处断言「env 不存在时回落 default」用 PX_NONEXISTENT_ENV ⇒ 必须保证它**没被设上**。
#   （这两条只出现在 0.2.0 用例里，0.1.0 不依赖 env。）它们是用例**明写的契约**，不是我方放宽判据。
export PX_TEST_HOST="env-host"
export APP_DB_HOST="env-db-host"
unset PX_NONEXISTENT_ENV 2>/dev/null || true
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

    # ---- 网络 fixture（M201 建 · M210 加 imap）：四个 mock 服务端（**普贤自建**，见 upstream-tests/fixtures/README.md）
    #   上游 ftp/pop3/oauth2 三个用例要求外部服务端（上游当时用 pyftpdlib / 手搭进程，未入库）。
    #   本仓纪律 = **不引入新依赖**（不依赖 python3/pyftpdlib）⇒ 用普贤自己写 mock 服务端：
    #     oauth2_mock 19090 · pop3_mock 2110 · ftp_mock 2121(控制)/2122(数据) · imap_mock 1143
    #   ⚠️ 只编**编译轨**（服务端是基础设施；且 spawn 并发仅编译轨支持 —— ftp 用例会并发开两条控制连接）。
    #   ⚠️ 在 $WORK 里编译（不在仓库树里生成 build/ —— 保持 git 干净、也不进 MANIFEST 判据）。
    if [ -d "$TESTS_DIR/fixtures" ]; then
        mkdir -p "$WORK/fixtures"
        cp -p "$TESTS_DIR"/fixtures/*.px "$WORK/fixtures/" 2>/dev/null || true
        NET_OK=0
        for s in oauth2_mock pop3_mock ftp_mock imap_mock; do
            [ -f "$WORK/fixtures/$s.px" ] || continue
            if ! ( cd "$WORK/fixtures" && "$PX" build "$s.px" ) >"$WORK/fixtures/$s.build.log" 2>&1; then
                echo "  ⚠ 网络 fixture $s 编译失败（依赖它的用例会失败）：$(tail -2 "$WORK/fixtures/$s.build.log" | tr '\n' ' ')"
                continue
            fi
            # ⚠️ 必须 `exec`：否则 `$!` 是**子 shell** 的 pid，cleanup 杀掉它而**服务端进程存活**
            #   （首版就踩了 —— 收尾后 2110/2121/19090 仍被占，下一轮 fixture 起不来）。
            ( cd "$WORK/fixtures" && exec "./build/$s" ) >"$WORK/fixtures/$s.log" 2>&1 &
            NET_PIDS+=($!)
            # 等它自报监听（最多 5s）；失败只警告 —— 判据交给用例本身（连接失败=FAIL，不静默）
            for _ in $(seq 1 50); do
                grep -q "listening" "$WORK/fixtures/$s.log" 2>/dev/null && break
                sleep 0.1
            done
            NET_OK=$((NET_OK + 1))
        done
        [ "$NET_OK" != 0 ] && echo "── 网络 fixture：$NET_OK 个 mock 服务端已起（oauth2 19090 · pop3 2110 · ftp 2121/2122 · imap 1143+2143）"
    fi
fi
echo "── 环境 fixture：PX_TEST_HOST=env-host · APP_DB_HOST=env-db-host（PX_NONEXISTENT_ENV 已 unset）"
: > "${JSON:-/dev/null}"

# ---------- 跑 ----------
PASS=0; FAIL=0; SKIP=0; XFAIL=0; MISMATCH=0; FLAKE=0
CONFIRM=${CONFIRM:-1}   # 失败确认步（M287s2）：1=启用（默认）／0=关闭（仅用于定性排查）
: > /tmp/.ut_res.$$
run_one() {  # $1=track  $2=test  → 设 RC / OUT
    local trk="$1" t="$2" log="$WORK/$1-$2.log"
    if [ "$trk" = build ]; then
        ( cd "$WORK/tests" && timeout "$TIMEOUT" "$PX" build "$t.px" ) >"$log.b" 2>&1
        local brc=$?
        if [ $brc -ne 0 ]; then RC=$brc; OUT="$(tail -3 "$log.b")"; return; fi
        # M282（R1）：**跑产物时 cwd 必须是 $WORK**，不是 $WORK/tests ——
        #   上游 0.2.0 用例改用**相对路径**访问 registry（如 walk2 的 wk_walk("registry/walk")），
        #   而 $WORK/registry 才是指向被测 registry 的软链。cwd=$WORK/tests 时该相对路径不存在
        #   ⇒ 用例报 "stat 失败: registry/walk"（**不是**库缺陷，是跑法不对）。
        #   构建仍在 $WORK/tests 里做 ⇒ 产物落在 $WORK/tests/build/<name>，故用 ./tests/build/…
        ( cd "$WORK" && timeout "$TIMEOUT" "./tests/build/$t" ) >"$log" 2>&1
        RC=$?
    else
        ( cd "$WORK" && timeout "$TIMEOUT" "$PX" run "tests/$t.px" ) >"$log" 2>&1
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
        # 成功判据（M282 修订）：rc==0 **且** 输出里出现「完成标记」（PASS 或 done）。
        #   ⚠️ 为什么加 `done`：原判据要求字面 `PASS` —— 那是**0.1.0 用例的书写习惯**，
        #      不是契约。0.1.0 的 125 个用例全部含 `PASS`，而 0.2.0 的 113 个里
        #      `barcode2_test` 打印的是 `barcode 0.2.0 tests done`（**没有** PASS）
        #      ⇒ 原判据会把它误判成 FAIL（假红）。
        #   ⚠️⚠️ **不要**改成「rc==0 且输出无 FAIL」—— 本轮实测踩过：`testkit_test` 的**本职**
        #      就是验证「断言失败时的报告」，它会**故意**打印 `FAIL: testkit_test（7/24 失败）`，
        #      再打印真正的结论 `testkit: PASS（成功/失败路径均符合预期）`，且 rc==0。
        #      ⇒ 「输出含 FAIL ⇒ 失败」这个**我猜的**契约是错的（实现里没有这条）。
        #   ⚠️ 完成标记是**书写习惯**、rc 才是**硬契约**：所以先判 rc（124=TIMEOUT / ≠0=FAIL），
        #      标记只用来排除「rc==0 但根本没跑到结尾」。
        #   ⚠️ 用例**自我跳过**的（如 `redis2_test` 在无 6379 时打印 `SKIP: …` 后 exit 0）
        #      在 EXPECTED.tsv 里按 **SKIP** 登记（不登记成 PASS），**不**在这里放宽。
        if [ "$RC" = 0 ] && printf '%s' "$OUT" | grep -qE 'PASS|done'; then
            got=PASS
        elif [ "$RC" = 124 ]; then
            got=TIMEOUT
        else
            got=FAIL
        fi
        # ── M287s2：**失败确认步**（先例：M207 gcstress_sweep.sh「FAIL 先自证稳定」）──
        #   动机（实测）：上游 `mock_test` 的 `mk_start_server` = `spawn mk_server_loop(...)`
        #   后**立即返回**（listener 在协程里创建）⇒ 客户端靠 `req_retry`（20×20ms）兜住
        #   启动竞态；而 `max_conns=10` 会被重试产生的**额外连接**吃掉 ⇒ 重负载下后续
        #   `mk_request` 撞 ECONNREFUSED。全量门内 1 次红；单独跑 2/2 全绿（419/0）。
        #   ⇒ 上游用例的**负载敏感设计**，不是 PuXian 缺陷。
        #   ⚠️ 只对「期望 PASS 却 FAIL」重跑：**真缺陷两次都失败 ⇒ 仍是 FAIL**，不被掩盖。
        #   ⚠️ 抖动**响亮计数**（汇总 + --json + ≥3 判非零），不静默。
        if [ "$got" = FAIL ] && [ "$ex" = PASS ] && [ "$CONFIRM" = 1 ]; then
            echo "  … $t[$trk] 首跑失败 ⇒ 确认步（重跑一次）"
            run_one "$trk" "$t"
            if [ "$RC" = 0 ] && printf '%s' "$OUT" | grep -qE 'PASS|done'; then
                got=FLAKE
            fi
        fi
        if [ "$ex" = SKIP ]; then   # --run-skipped 下跑了期望跳过的：只作信息
            [ "$VERBOSE" = 1 ] && echo "  ℹ $t[$trk] 期望 SKIP，实跑 $got"
        elif [ "$ex" = XFAIL ]; then
            # M282：**期望失败**（上游 0.2.0 用例/库自相矛盾 —— 见 docs/UPSTREAM_020_DEFECTS.md）
            #   反向判据：一旦它**通过** ⇒ 上游可能已修 ⇒ **判红**，逼我们回来复核登记。
            #   ⇒ XFAIL 不是"把红记成绿"的出口，而是"把已知上游缺陷显式登记 + 自动检测其修复"。
            if [ "$got" = PASS ]; then
                echo "  ✗ $t[$trk] 登记为 XFAIL 但**实测通过** ⇒ 上游可能已修，请复核并更新 EXPECTED.tsv"
                FAIL=$((FAIL + 1))
            else
                XFAIL=$((XFAIL + 1))
                [ "$VERBOSE" = 1 ] && echo "  ⊘ $t[$trk] 期望失败（上游缺陷）· 实跑 $got"
            fi
        elif [ "$ex" = PASS ] && [ "$got" = FLAKE ]; then
            # M287s2：**抖动**（首跑失败、确认步通过）—— 响亮计数，**不判红**
            echo "  ⚠ $t[$trk] 抖动：首跑失败、确认步通过（负载敏感，见脚本头注）"
            FLAKE=$((FLAKE + 1))
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
echo "  通过 $PASS · 失败 $FAIL · 跳过 $SKIP · 期望失败(XFAIL) $XFAIL · 期望值缺失 $MISMATCH · 抖动(FLAKE) $FLAKE"
if [ -n "$JSON" ]; then
    # ⚠️ SKIP 行的第 4 列是**理由**（不是 rc）⇒ 生成 JSON 时必须区分，
    #   否则产出 `"rc":interp 设计性不支持 …`（非法 JSON —— 首版就踩了这个）。
    { echo "{"; echo '  "tests": [';
      awk -F'\t' 'BEGIN{n=0} {n++; rc=($4 ~ /^-?[0-9]+$/ ? $4 : "null");
        printf "%s    {\"test\":\"%s\",\"track\":\"%s\",\"result\":\"%s\",\"rc\":%s}", (n>1?",\n":""), $1,$2,$3,rc} END{print ""}' /tmp/.ut_res.$$;
      echo "  ],"; echo "  \"pass\": $PASS, \"fail\": $FAIL, \"skip\": $SKIP, \"xfail\": $XFAIL, \"mismatch\": $MISMATCH, \"flake\": $FLAKE"; echo "}"; } > "$JSON"
fi
rm -f /tmp/.ut_res.$$
if [ "$FLAKE" != 0 ] && [ "$FAIL" = 0 ] && [ "$MISMATCH" = 0 ]; then
    echo "══ 注意：与 EXPECTED 一致，但有 $FLAKE 次**抖动**（首跑失败、确认步通过）══"
fi
if [ "$FLAKE" -ge 3 ] && [ "$FAIL" = 0 ]; then
    echo "   ⚠️ 抖动 $FLAKE 次（≥3）—— 不再是「偶发」，请查负载/端口争用"
fi
if [ "$FAIL" != 0 ] || [ "$MISMATCH" != 0 ] || [ "$FLAKE" -ge 3 ]; then
    echo "══ 汇总：上游用例回归 与 EXPECTED 不一致 ══"
    [ "$KEEP" = 1 ] && echo "工作区保留: $WORK"
    exit 1
fi
echo "══ 汇总：上游用例回归 全部符合 EXPECTED ══"
[ "$KEEP" = 1 ] && echo "工作区保留: $WORK"
exit 0
