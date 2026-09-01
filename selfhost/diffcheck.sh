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

# ---- 入口 ----
if [ "${1:-}" = "--parser" ]; then
    shift
    if [ -n "${1:-}" ]; then
        check_parser_file "$1"
    else
        for f in "$(dirname "$0")"/cases/*.px; do
            [ -e "$f" ] && check_parser_file "$f"
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
            [ -e "$f" ] && check_lexer_file "$f" "$LEXER_MODE"
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
    echo "用法: $0 <file.px> | --all | --lexer [--build] [file.px]"
    exit 1
fi
