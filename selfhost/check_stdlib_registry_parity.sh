#!/usr/bin/env bash
# ============================================================
# selfhost/check_stdlib_registry_parity.sh —— stdlib ⇄ registry **内容对拍**（M381 建立）
# ------------------------------------------------------------
# 判据：对每个「stdlib/<n>.px 存在、registry/<n>/ 也存在」的包，
#       取 registry 的**最高版本目录**，其中每个 `.px` 必须能在 stdlib 找到同名文件
#       且**逐字节一致**（cmp）；反向：registry 里出现 stdlib 没有的文件 = 幽灵文件 ⇒ 红。
#
# 为什么要有它（M377 记的债，M381 才补）：
#   · `stdlib/pxml.px` 与 `registry/pxml/0.1.0/pxml.px` 只差 1 行注释，**没有任何门在比**
#     ⇒ 真实漂移面：M381 实测 collections / cookiejar / yaml 三个包的 registry 副本
#       停留在「修复前」的旧内容（chunk 死循环、cj_is_int 守卫、yaml 三文件拆分都没进 registry）
#       ⇒ pxpkg 装出来的包与 stdlib 行为不同，而**没有任何判据**看得见。
#   · 只比「最高版本」是有意的：registry 的版本目录是**发布历史**，
#     旧版本与新版本不同是常态；要对拍的是「当前发布物 = stdlib 现状」。
#
# 豁免表 selfhost/registry_parity.allow（每行 `包名|理由`）：
#   ① 每条必须有理由（空理由 ⇒ 红）；② **豁免过期即红**（实际已一致却还豁免）；
#   ③ 悬空豁免（包名不在配对里）⇒ 红。
#   ⇒ 白名单只能「暂时挂账」，不能变成永久后门。
#
# 规模锚点：配对数量必须 ≥ $MIN_PAIRS（默认 10），否则判红
#   —— 防「扫描器坏了 ⇒ 空集 ⊆ 空集 ⇒ 全绿」。
#
# 用法：
#   bash selfhost/check_stdlib_registry_parity.sh                 # 门模式
#   bash selfhost/check_stdlib_registry_parity.sh --self-test     # 负控自测（隔离临时树）
#   REPO_ROOT=/tmp/x bash selfhost/... --check-only               # 对指定根求值（自测用）
# 退出码：0 = 绿 · 1 = 红 · 2 = 前置自查失败（自测不过/参数错）
# ============================================================
set -uo pipefail
export LC_ALL=C LANG=C
HERE="$(cd "$(dirname "$0")" && pwd)"
SELF="$HERE/$(basename "$0")"
MIN_PAIRS="${MIN_PAIRS:-10}"

# ── 核心求值：$1 = 仓库根；$2 = allow 文件（可为不存在）──
run_check() {
    local repo="$1" allow="$2"
    local pass=0 fail=0 pairs=0
    local -a names=() states=()
    ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
    bad() { echo "  ❌ $1"; fail=$((fail+1)); }

    echo "=== stdlib ⇄ registry 内容对拍（M381）==="
    echo "  根：$repo · 规模锚点：配对 ≥ $MIN_PAIRS"

    local f n v dir b
    for f in "$repo"/stdlib/*.px; do
        [ -f "$f" ] || continue
        n="$(basename "$f" .px)"
        [ -d "$repo/registry/$n" ] || continue
        v="$(ls "$repo/registry/$n" 2>/dev/null | sort -V | tail -1)"
        [ -n "$v" ] || { bad "$n：registry/$n/ 下没有版本目录"; continue; }
        dir="$repo/registry/$n/$v"
        pairs=$((pairs+1))
        if [ ! -f "$dir/$n.px" ]; then
            bad "$n：最高版 $v 缺入口 $n.px"
            names+=("$n"); states+=("DIFF"); continue
        fi
        local st="SAME" detail=""
        for b in "$dir"/*.px; do
            [ -f "$b" ] || continue
            local bn; bn="$(basename "$b")"
            if [ ! -f "$repo/stdlib/$bn" ]; then
                st="DIFF"; detail="$detail 幽灵文件:$bn"
            elif ! cmp -s "$b" "$repo/stdlib/$bn"; then
                st="DIFF"; detail="$detail 内容不同:$bn"
            fi
        done
        names+=("$n"); states+=("$st")
        if [ "$st" = "SAME" ]; then
            echo "  · $n @ $v ✅ 逐字节一致"
        else
            echo "  · $n @ $v ⚠️ 不一致：$detail"
        fi
    done

    # ── 规模锚点 ──
    if [ "$pairs" -ge "$MIN_PAIRS" ]; then
        ok "规模锚点：配对 $pairs 个（≥ $MIN_PAIRS）"
    else
        bad "规模锚点失守：配对仅 $pairs 个（< $MIN_PAIRS）—— 扫描器可能坏了，不允许全绿"
    fi

    # ── 豁免表 ──
    local -A allowmap=()
    local allown=0
    if [ -f "$allow" ]; then
        local line nm rs
        while IFS= read -r line; do
            case "$line" in ''|'#'*) continue ;; esac
            nm="${line%%|*}"
            rs="${line#*|}"
            if [ "$nm" = "$line" ] || [ -z "$(echo "$rs" | tr -d ' ')" ]; then
                bad "豁免表条目缺理由：$line"
                continue
            fi
            allowmap["$nm"]="$rs"; allown=$((allown+1))
        done < "$allow"
    fi
    echo "  豁免表：$allown 条（$allow）"

    local i nm2 st2 rs2
    for i in "${!names[@]}"; do
        nm2="${names[$i]}"; st2="${states[$i]}"
        if [ "$st2" = "DIFF" ] && [ -z "${allowmap[$nm2]:-}" ]; then
            bad "$nm2：registry 最高版与 stdlib 不一致，且未登记豁免"
        elif [ "$st2" = "SAME" ] && [ -n "${allowmap[$nm2]:-}" ]; then
            bad "$nm2：豁免已过期（实际已一致）—— 请从 allow 表删除"
        fi
    done
    # 悬空豁免
    if [ "${#allowmap[@]}" -gt 0 ]; then
        for nm2 in "${!allowmap[@]}"; do
            local found=0
            for i in "${!names[@]}"; do [ "${names[$i]}" = "$nm2" ] && found=1; done
            [ "$found" = 1 ] || bad "豁免表悬空：$nm2 不在 stdlib/registry 配对里"
        done
    fi

    echo "── 合计：配对 $pairs · PASS $pass · FAIL $fail"
    if [ "$fail" -eq 0 ]; then
        echo "STDLIB-REGISTRY-PARITY-OK"
        return 0
    fi
    echo "STDLIB-REGISTRY-PARITY-FAIL"
    return 1
}

# ── 自测：负控必须有牙 ──
self_test() {
    local T; T="$(mktemp -d)"
    local sp=0 sf=0
    sok() { echo "  ✅ $1"; sp=$((sp+1)); }
    sbad() { echo "  ❌ $1"; sf=$((sf+1)); }

    mkdir -p "$T/stdlib" "$T/registry/foo/0.1.0" "$T/registry/bar/0.2.0"
    printf 'def foo():\n    return 1\n' > "$T/stdlib/foo.px"
    cp "$T/stdlib/foo.px" "$T/registry/foo/0.1.0/foo.px"
    printf 'def bar():\n    return 2\n' > "$T/stdlib/bar.px"
    cp "$T/stdlib/bar.px" "$T/registry/bar/0.2.0/bar.px"

    # A 前提自证：扰动确实改变了内容（否则后面的负控测不到东西）
    local before after
    before="$(sha256sum "$T/stdlib/foo.px" | cut -c1-16)"
    echo '# x' >> "$T/registry/foo/0.1.0/foo.px"
    after="$(sha256sum "$T/registry/foo/0.1.0/foo.px" | cut -c1-16)"
    if [ "$before" != "$after" ]; then sok "前提自证：扰动使 sha 变化（$before → $after）"
    else sbad "前提自证失败：扰动未改变内容，负控无意义"; fi

    # B 正控：一致 ⇒ 绿
    cp "$T/stdlib/foo.px" "$T/registry/foo/0.1.0/foo.px"
    local out rc
    out="$(MIN_PAIRS=1 REPO_ROOT="$T" bash "$SELF" --check-only 2>&1)"; rc=$?
    if [ "$rc" = 0 ] && grep -q "PARITY-OK" <<< "$out"; then sok "正控：一致 ⇒ rc=0"
    else sbad "正控失败：rc=$rc"; echo "$out" | tail -5; fi

    # C 负控：改一个字节 ⇒ 红且点名
    echo '# drift' >> "$T/registry/foo/0.1.0/foo.px"
    out="$(MIN_PAIRS=1 REPO_ROOT="$T" bash "$SELF" --check-only 2>&1)"; rc=$?
    if [ "$rc" = 1 ] && grep -q "foo：registry 最高版与 stdlib 不一致" <<< "$out"; then
        sok "负控 1：内容漂移 ⇒ rc=1 且点名 foo"
    else sbad "负控 1 失败：rc=$rc（判据没牙）"; echo "$out" | tail -5; fi

    # D 负控：豁免过期 ⇒ 红
    cp "$T/stdlib/foo.px" "$T/registry/foo/0.1.0/foo.px"
    printf 'foo|暂挂（自测故意过期）\n' > "$T/allow.txt"
    out="$(MIN_PAIRS=1 REPO_ROOT="$T" ALLOW_OVERRIDE="$T/allow.txt" bash "$SELF" --check-only 2>&1)"; rc=$?
    if [ "$rc" = 1 ] && grep -q "豁免已过期" <<< "$out"; then
        sok "负控 2：豁免过期 ⇒ rc=1"
    else sbad "负控 2 失败：rc=$rc"; echo "$out" | tail -5; fi

    # E 负控：规模锚点失守 ⇒ 红
    rm -rf "$T/stdlib"; mkdir -p "$T/stdlib"
    out="$(MIN_PAIRS=10 REPO_ROOT="$T" bash "$SELF" --check-only 2>&1)"; rc=$?
    if [ "$rc" = 1 ] && grep -q "规模锚点失守" <<< "$out"; then
        sok "负控 3：配对为 0 ⇒ rc=1（防空集假绿）"
    else sbad "负控 3 失败：rc=$rc"; echo "$out" | tail -5; fi

    echo "── 自测：PASS $sp · FAIL $sf"
    rm -rf "$T"
    [ "$sf" -eq 0 ] && { echo "PARITY-SELFTEST-OK"; return 0; }
    echo "PARITY-SELFTEST-FAIL"; return 1
}

case "${1:-}" in
    --self-test)
        self_test || exit 2
        exit 0 ;;
    --check-only)
        REPO_ROOT="${REPO_ROOT:-$HERE/..}"
        ALLOW="${ALLOW_OVERRIDE:-$REPO_ROOT/selfhost/registry_parity.allow}"
        run_check "$REPO_ROOT" "$ALLOW"
        exit $? ;;
    '')
        self_test >/dev/null 2>&1 || { echo "❌ 前置自查失败：--self-test 不过，先修门再做判据" >&2; exit 2; }
        run_check "$HERE/.." "$HERE/registry_parity.allow"
        exit $? ;;
    *)
        echo "未知参数：$1" >&2; exit 2 ;;
esac
