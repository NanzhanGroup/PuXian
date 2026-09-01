#!/usr/bin/env bash
# ============================================================
# PuXian 自举对拍框架（M-B1 + M-B2）
# ------------------------------------------------------------
# 用途：同一输入，Rust 版编译器 vs PuXian 版编译器输出逐字节一致
#   lex   → token 流
#   parse → AST dump
#   build → C 源码
#   run   → stdout
# ------------------------------------------------------------
# 用法：
#   ./diffcheck.sh <file.px>          # 基线模式：Rust 版输出存 golden（PuXian 版未就绪时）
#   ./diffcheck.sh <file.px> --check  # 对拍模式：Rust 版 vs PuXian 版（自举编译器就绪后）
#   ./diffcheck.sh --all              # 对拍 cases/ 下全部用例
#   ./diffcheck.sh --lexer [--build] [file.px]  # M-B2：PuXian lexer 对拍 token 流
#   ./diffcheck.sh --parser [file.px]          # M-B3：PuXian parser 对拍 AST
#   ./diffcheck.sh --errors                    # M-B4：错误场景对拍（cases_bad/）
# ------------------------------------------------------------
# 对拍契约（Mini 子集规范 §五）：
#   lex   ：token 序列规范化后逐 token 一致（去行首空白/压缩连续空格/保留顺序与值）
#   parse ：AST dump 规范化后一致（PuXian 版输出同构 AST 文本）
#   build ：生成 C 源码逐字节一致（不含绝对路径注释）
#   run   ：stdout 逐字节一致
# ============================================================
set -u

PX="$(dirname "$0")/../compiler/target/release/px"
PXC="${PXC:-}"   # PuXian 版编译器（未来自举产物，如 selfhost/build/pxc）
GOLDEN_DIR="$(dirname "$0")/golden"
WORK="/tmp/px_diffcheck"
mkdir -p "$WORK" "$GOLDEN_DIR"

norm_tokens() {
    # 规范化 token 流：去行首空白、压缩连续空格、去尾部空白、去"共 N 个 Token"行
    sed -E 's/^[[:space:]]+//; s/[[:space:]]+/ /g; s/[[:space:]]+$//' \
        | grep -vE '^共 [0-9]+ 个 Token$' \
        | grep -vE '^$'
}

norm_ast() {
    # 规范化 AST dump：去空白差异（保留结构）
    sed -E 's/[[:space:]]+/ /g; s/[[:space:]]+$//' \
        | grep -vE '^$'
}

norm_c() {
    # 规范化 C 源码：去掉生成注释头（含路径）与行尾空白
    grep -vE '^/\* 由普贤' \
        | sed -E 's/[[:space:]]+$//'
}

# ---- M-B4：错误场景对拍 ----
#   同一坏输入，Rust 版 px lex/parse 首行错误消息 vs PuXian 版 lexer/parser 首行错误消息
#   契约：PuXian 版 err/perr 输出格式与 Rust 版逐字节一致
#     lex   ：错误: pos: 词法错误 code: msg（Rust px lex 带前缀）
#     parse ：pos: 语法错误 code: msg（Rust px parse 不带前缀）
check_error_file() {
    local f="$1" kind="$2"
    local base; base=$(basename "$f")
    local rust_out px_out
    if [ "$kind" = "lex" ]; then
        rust_out=$("$PX" lex "$f" 2>&1 | head -1)
        px_out=$("$PX" run "$(dirname "$0")/lexer.px" "$f" 2>&1 | head -1)
    else
        rust_out=$("$PX" parse "$f" 2>&1 | head -1)
        px_out=$("$PX" run "$(dirname "$0")/parser.px" "$f" 2>&1 | head -1)
    fi
    if [ "$rust_out" = "$px_out" ]; then
        echo "    ✅ $base"
    else
        echo "    ❌ $base"
        echo "      Rust: $rust_out"
        echo "      PX  : $px_out"
        return 1
    fi
    return 0
}

# ---- M-B3：PuXian parser 对拍 ----
#   解释器执行 selfhost/parser.px <file>，AST dump 与 golden/*.ast 对比
check_parser_file() {
    local f="$1"
    local base
    base="$(basename "$f" .px)"
    echo "── parser 对拍: $f"
    "$PX" run "$(dirname "$0")/parser.px" "$f" 2>&1 | norm_ast > "$WORK/$base.px.ast"
    norm_ast < "$GOLDEN_DIR/$base.ast" > "$WORK/$base.gold.ast" 2>/dev/null || { echo "    ⚠️ 无 golden，先生成"; return; }
    if diff -q "$WORK/$base.px.ast" "$WORK/$base.gold.ast" >/dev/null 2>&1; then
        echo "    ✅ $base AST 一致"
    else
        echo "    ❌ $base 有差异"
        diff "$WORK/$base.px.ast" "$WORK/$base.gold.ast" | head -10
    fi
}

# ---- M-B2：PuXian lexer 对拍 ----
#   run 模式：解释器执行 selfhost/lexer.px <file>
#   build 模式：编译版 selfhost/build/lexer <file>
check_lexer_file() {
    local f="$1" mode="$2"
    local base
    base="$(basename "$f" .px)"
    echo "── lexer 对拍: $f ($mode)"
    if [ "$mode" = "build" ]; then
        "$(dirname "$0")/build/lexer" "$f" 2>&1 | norm_tokens > "$WORK/$base.px.tokens"
    else
        "$PX" run "$(dirname "$0")/lexer.px" "$f" 2>&1 | norm_tokens > "$WORK/$base.px.tokens"
    fi
    norm_tokens < "$GOLDEN_DIR/$base.tokens" > "$WORK/$base.gold.tokens"
    if diff -q "$WORK/$base.px.tokens" "$WORK/$base.gold.tokens" >/dev/null 2>&1; then
        echo "    ✅ $base token 流一致"
    else
        echo "    ❌ $base 有差异"
        diff "$WORK/$base.px.tokens" "$WORK/$base.gold.tokens" | head -10
    fi
}

check_file() {
    local f="$1"
    local base
    base="$(basename "$f" .px)"
    echo "── 对拍: $f"

    # ---- 1. lex：Rust 版 token 流 ----
    "$PX" lex "$f" 2>&1 | norm_tokens > "$WORK/$base.tokens"
    # ---- 2. parse：Rust 版 AST dump ----
    "$PX" parse "$f" 2>&1 | norm_ast > "$WORK/$base.ast"
    # ---- 3. build：C 源码 ----
    local csrc="$WORK/$base.c"
    if "$PX" build "$f" >/dev/null 2>&1; then
        local bdir
        bdir="$(dirname "$f")/build"
        norm_c < "$bdir/$base.c" > "$csrc" 2>/dev/null || echo "" > "$csrc"
    else
        echo "    [build] 编译失败（记录为空）"
        echo "" > "$csrc"
    fi
    # ---- 4. run：stdout ----
    "$PX" run "$f" > "$WORK/$base.stdout" 2>&1

    if [ -n "$PXC" ]; then
        # ================= 对拍模式：Rust vs PuXian =================
        local ok=1
        # lex
        "$PXC" lex "$f" 2>&1 | norm_tokens > "$WORK/$base.pxc.tokens"
        if ! diff -q "$WORK/$base.tokens" "$WORK/$base.pxc.tokens" >/dev/null 2>&1; then
            echo "    [FAIL] lex token 流不一致"
            diff "$WORK/$base.tokens" "$WORK/$base.pxc.tokens" | head -10
            ok=0
        else
            echo "    [OK]   lex"
        fi
        # parse
        "$PXC" parse "$f" 2>&1 | norm_ast > "$WORK/$base.pxc.ast"
        if ! diff -q "$WORK/$base.ast" "$WORK/$base.pxc.ast" >/dev/null 2>&1; then
            echo "    [FAIL] parse AST 不一致"
            diff "$WORK/$base.ast" "$WORK/$base.pxc.ast" | head -10
            ok=0
        else
            echo "    [OK]   parse"
        fi
        # build → C
        if "$PXC" build "$f" >/dev/null 2>&1; then
            local pbdir
            pbdir="$(dirname "$f")/build"
            norm_c < "$pbdir/$base.c" > "$WORK/$base.pxc.c" 2>/dev/null || echo "" > "$WORK/$base.pxc.c"
        else
            echo "" > "$WORK/$base.pxc.c"
        fi
        if ! diff -q "$WORK/$base.c" "$WORK/$base.pxc.c" >/dev/null 2>&1; then
            echo "    [FAIL] C 源码不一致"
            diff "$WORK/$base.c" "$WORK/$base.pxc.c" | head -10
            ok=0
        else
            echo "    [OK]   codegen(C)"
        fi
        # run
        if [ -x "$(dirname "$PXC")/$(basename "$f" .px)" ]; then
            "$(dirname "$PXC")/$(basename "$f" .px)" > "$WORK/$base.pxc.stdout" 2>&1
            if ! diff -q "$WORK/$base.stdout" "$WORK/$base.pxc.stdout" >/dev/null 2>&1; then
                echo "    [FAIL] stdout 不一致"
                diff "$WORK/$base.stdout" "$WORK/$base.pxc.stdout" | head -10
                ok=0
            else
                echo "    [OK]   run(stdout)"
            fi
        fi
        [ "$ok" = "1" ] && echo "    ✅ $base 双模式一致" || echo "    ❌ $base 有差异"
    else
        # ================= 基线模式：保存 golden =================
        cp "$WORK/$base.tokens" "$GOLDEN_DIR/$base.tokens"
        cp "$WORK/$base.ast"    "$GOLDEN_DIR/$base.ast"
        cp "$WORK/$base.c"      "$GOLDEN_DIR/$base.c"
        cp "$WORK/$base.stdout" "$GOLDEN_DIR/$base.stdout"
        echo "    ✅ 基线已保存 golden/$base.{tokens,ast,c,stdout}"
    fi
}

# ---- M-B6：PuXian codegen 对拍 ----
#   解释器执行 selfhost/codegen.px <file> 输出 C 源码，与 golden/*.c 对比
#   --build：编译版 selfhost/build/codegen <file>（验证 codegen.px 在 Mini 子集内）
check_codegen_file() {
    local f="$1" mode="$2"
    local base
    base="$(basename "$f" .px)"
    echo "── codegen 对拍: $f ($mode)"
    local out
    if [ "$mode" = "build" ]; then
        out=$("$(dirname "$0")/build/codegen" "$f" 2>&1)
    else
        out=$("$PX" run "$(dirname "$0")/codegen.px" "$f" 2>&1)
    fi
    norm_c <<< "$out" > "$WORK/$base.px.c"
    norm_c < "$GOLDEN_DIR/$base.c" > "$WORK/$base.gold.c" 2>/dev/null || { echo "    ⚠️ 无 golden，先生成"; return; }
    if diff -q "$WORK/$base.px.c" "$WORK/$base.gold.c" >/dev/null 2>&1; then
        echo "    ✅ $base C 源码一致"
    else
        echo "    ❌ $base C 源码有差异"
        diff "$WORK/$base.px.c" "$WORK/$base.gold.c" | head -10
    fi
}

# ---- M-B5：值系统/作用域/模块对拍 ----
#   解释器执行 value/env/module 用例（v0*.px），断言全 [PASS] 无 [FAIL]/运行时错误
#   --build：额外编译并运行编译版（双模式验证）
prepare_m5_modtest() {
    # v03_module.px 依赖的临时目录结构（对齐 Rust module.rs 测试场景）
    local d=/tmp/px_m5_modtest
    if [ ! -d "$d" ]; then
        mkdir -p "$d/pkg" "$d/lib" "$d/stdlib"
        echo 'def mylib_fn(): return 1' > "$d/mylib.px"
        echo 'def tool(): return 2' > "$d/pkg/tools.px"
        echo 'def libfn(): return 3' > "$d/lib/mod.px"
        echo 'def unique(): return 4' > "$d/stdlib/collections.px"
    fi
}

check_value_case() {
    local f="$1" mode="$2"
    local base; base="$(basename "$f" .px)"
    echo "── value 对拍: $f ($mode)"
    local out
    if [ "$mode" = "build" ]; then
        local bdir; bdir="$(dirname "$f")/build"
        if ! "$PX" build "$f" >/dev/null 2>&1; then
            echo "    ❌ $base 编译失败"
            return 1
        fi
        out=$("$bdir/$base" 2>&1)
    else
        out=$("$PX" run "$f" 2>&1)
    fi
    local fails errs passes
    fails=$(echo "$out" | grep -cE '^\[FAIL\]' || true)
    errs=$(echo "$out" | grep -cE '运行时错误|错误 \[' || true)
    passes=$(echo "$out" | grep -cE '^\[PASS\]' || true)
    if [ "$fails" = "0" ] && [ "$errs" = "0" ]; then
        echo "    ✅ $base $passes PASS"
        return 0
    else
        echo "    ❌ $base：$fails FAIL / $errs 错误"
        echo "$out" | grep -E '^\[FAIL\]|运行时错误|错误 \[' | head -5
        return 1
    fi
}

# ---- M-B7：interp 对拍（tree-walking 解释器） ----
#   run 模式：解释器执行 selfhost/interp.px <file>（Rust px 解释执行 interp）
#   build 模式：编译版 selfhost/build/interp <file>（验证 interp.px 在 Mini 子集内）
# 对拍契约（MINI_SUBSET §五 run 层）：
#   s0*.px → stdout 与 golden/*.stdout 逐字节一致
#   v0*.px → 全 [PASS] 无 [FAIL]/运行时错误
# 已知差异：s08（语法覆盖用例，Rust 版本身运行报错，非 run 用例）；--build 的 v01（编译模式浮点 %g 精度）
check_interp_stdout() {
    local f="$1" mode="$2"
    local base; base="$(basename "$f" .px)"
    echo "── interp 对拍: $f ($mode)"
    local out
    if [ "$mode" = "build" ]; then
        out=$("$(dirname "$0")/build/interp" "$f" 2>&1)
    else
        out=$("$PX" run "$(dirname "$0")/interp.px" "$f" 2>&1)
    fi
    echo "$out" > "$WORK/$base.px.stdout"
    if diff -q "$WORK/$base.px.stdout" "$GOLDEN_DIR/$base.stdout" >/dev/null 2>&1; then
        echo "    ✅ $base stdout 一致"
        return 0
    else
        echo "    ❌ $base stdout 有差异"
        diff "$GOLDEN_DIR/$base.stdout" "$WORK/$base.px.stdout" | head -6
        return 1
    fi
}

check_interp_value() {
    local f="$1" mode="$2"
    local base; base="$(basename "$f" .px)"
    echo "── interp 对拍: $f ($mode)"
    local out
    if [ "$mode" = "build" ]; then
        out=$("$(dirname "$0")/build/interp" "$f" 2>&1)
    else
        out=$("$PX" run "$(dirname "$0")/interp.px" "$f" 2>&1)
    fi
    local fails errs passes
    fails=$(echo "$out" | grep -cE '^\[FAIL\]' || true)
    errs=$(echo "$out" | grep -cE '运行时错误|错误 \[' || true)
    passes=$(echo "$out" | grep -cE '^\[PASS\]' || true)
    if [ "$fails" = "0" ] && [ "$errs" = "0" ]; then
        echo "    ✅ $base $passes PASS"
        return 0
    else
        echo "    ❌ $base：$fails FAIL / $errs 错误"
        echo "$out" | grep -E '^\[FAIL\]|运行时错误|错误 \[' | head -4
        return 1
    fi
}

# ---- 入口 ----
if [ "${1:-}" = "--interp" ]; then
    shift
    INTERP_MODE="run"
    if [ "${1:-}" = "--build" ]; then INTERP_MODE="build"; shift; fi
    fail=0
    echo "── interp 对拍（M-B7，tree-walking 解释器）──"
    for f in "$(dirname "$0")"/cases/s*.px; do
        [ -e "$f" ] || continue
        case "$(basename "$f")" in
            s08_comprehensive.px) continue;;  # 语法覆盖用例，Rust 版本身运行报错（非 run 用例）
        esac
        check_interp_stdout "$f" "$INTERP_MODE" || fail=1
    done
    for f in "$(dirname "$0")"/cases/v0*.px; do
        [ -e "$f" ] || continue
        if [ "$INTERP_MODE" = "build" ] && [ "$(basename "$f")" = "v01_value.px" ]; then
            echo "── interp 对拍: $f (build，已知浮点 %g 精度差异，跳过)"
            continue
        fi
        check_interp_value "$f" "$INTERP_MODE" || fail=1
    done
    if [ "$fail" = "0" ]; then
        echo "interp 对拍全部通过 ✅"
        exit 0
    else
        echo "存在失败 ❌"
        exit 1
    fi
fi

if [ "${1:-}" = "--value" ]; then
    VAL_MODE="run"
    shift
    if [ "${1:-}" = "--build" ]; then VAL_MODE="build"; shift; fi
    prepare_m5_modtest
    fail=0
    echo "── 值系统对拍（M-B5，value/env/module）──"
    for f in "$(dirname "$0")"/cases/v0*.px; do
        [ -e "$f" ] || continue
        check_value_case "$f" "$VAL_MODE" || fail=1
    done
    if [ "$fail" = "0" ]; then
        echo "值系统对拍全部通过 ✅"
        exit 0
    else
        echo "存在失败 ❌"
        exit 1
    fi
fi

if [ "${1:-}" = "--errors" ]; then
    shift
    fail=0
    echo "── 错误场景对拍（M-B4，cases_bad/）──"
    for f in "$(dirname "$0")"/cases_bad/lex_b*.px; do
        [ -e "$f" ] && check_error_file "$f" lex || fail=1
    done
    for f in "$(dirname "$0")"/cases_bad/parse_b*.px; do
        [ -e "$f" ] && check_error_file "$f" parse || fail=1
    done
    if [ "$fail" = "0" ]; then
        echo "错误场景全部一致 ✅"
        exit 0
    else
        echo "存在不一致 ❌"
        exit 1
    fi
fi


if [ "${1:-}" = "--codegen" ]; then
    shift
    CODEGEN_MODE="run"
    if [ "${1:-}" = "--build" ]; then CODEGEN_MODE="build"; shift; fi
    if [ -n "${1:-}" ]; then
        check_codegen_file "$1" "$CODEGEN_MODE"
    else
        for f in "$(dirname "$0")"/cases/*.px; do
            [ -e "$f" ] || continue
            check_codegen_file "$f" "$CODEGEN_MODE"
        done
    fi
    exit 0
fi

if [ "${1:-}" = "--parser" ]; then
    shift
    if [ -n "${1:-}" ]; then
        check_parser_file "$1"
    else
        for f in "$(dirname "$0")"/cases/*.px; do
            [ -e "$f" ] || continue
            case "$(basename "$f")" in v0*.px) continue;; esac  # M-B5 用例走 --value
            check_parser_file "$f"
        done
    fi
    exit 0
fi

if [ "${1:-}" = "--lexer" ]; then
    LEXER_MODE="run"
    shift
    if [ "${1:-}" = "--build" ]; then LEXER_MODE="build"; shift; fi
    if [ -n "${1:-}" ]; then
        check_lexer_file "$1" "$LEXER_MODE"
    else
        for f in "$(dirname "$0")"/cases/*.px; do
            [ -e "$f" ] || continue
            case "$(basename "$f")" in v0*.px) continue;; esac  # M-B5 用例走 --value
            check_lexer_file "$f" "$LEXER_MODE"
        done
    fi
    exit 0
fi

if [ "${1:-}" = "--all" ]; then
    for f in "$(dirname "$0")"/cases/*.px; do
        [ -e "$f" ] || { echo "cases/ 下无用例"; exit 0; }
        check_file "$f"
    done
elif [ -n "${1:-}" ]; then
    check_file "$1"
else
    echo "用法: $0 <file.px> | --all | --lexer [--build] [file.px] | --parser [file.px] | --errors"
    exit 1
fi
