#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M387 · 缺陷 700：`px lint <dir>` 假绿（晨曦 2026-11-10 11:21 报的 P0）
# ---------------------------------------------------------------------------
# 病根（三段，全部实测过）：
#   ① tools/pxlint.px 没有「输入是目录」的分支 —— main() 把位置参数直接当文件喂给 lx_run
#   ② read_file(目录) 静默返回空串（同一份 pxi 读普通文件 len=28）
#   ③ file_size(目录) 反而返回 48（不能靠「读失败」判类型，必须靠 file_stat.is_dir）
#   ⇒ 空串 → 空 token 流 → 空程序 → 0 诊断 → exit(0)。**假绿是常规形态**，崩溃只是偶发形态。
#
# 判据（贴契约写，不贴表象）：
#   C1 目录模式输出的诊断集合 == 对目录下每个 .px 单文件跑 lint 的并集（按行排序比较）
#   C2 目录模式 rc：有错误 ⇒ ≠0；全干净 ⇒ 0
#   C3 「0 个 .px」与「0 个错误」必须可区分（空目标不算干净 ⇒ rc≠0 且 stderr 出声）
#   C4 单文件路径输出逐字节不变（旧契约不许被顺手改）
#   C5 --json 目录模式：条数 == 文本模式诊断条数，且 file 字段是全路径
#   C6 .git 跳过（不扫 VCS 内部）/ 隐藏目录照扫（否则「藏在 .foo/ 里的坏文件」又成假绿）
#   C7 读不到（悬空链接 ⇒ file_stat 返 null）※计错误并出声，不许静默跳过
#   C8 --strict：目录有警告 ⇒ rc=1；非 strict ⇒ rc=0
#   C9 硬词法错误（E1005）不许被吞成「0 错误」——rc 必须 ≠0
#   C10 性能上界：100 个 .px 的目录 < 60s（防「每文件重建 400+ 名册」退化）
#
# 负控（4 条，全部是本病症的真实形状；负控不红 ⇒ 判据没有牙）：
#   A 只跑第一个文件        ⇒ C1 必须红
#   B 删掉 is_dir 分支（目录当文件）⇒ C1 必须红
#   C 空目录当干净（exit 0）  ⇒ C3 必须红
#   D 静默跳过悬空链接        ⇒ C7 必须红
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PXBUILD="$ROOT/tools/px"
TMP="${TMPDIR:-/tmp}/m387_lint_dir.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
NEG_SRC=()
cleanup() {
    rm -rf "$TMP"
    local s
    for s in "${NEG_SRC[@]:-}"; do
        [ -n "$s" ] || continue
        rm -f "$ROOT/tools/$s" "$ROOT/tools/build/${s%.px}"
    done
}
trap cleanup EXIT

pass=0; fail=0
SHA0=$(sha256sum "$ROOT/tools/pxlint.px" 2>/dev/null | cut -d' ' -f1)
ok()   { echo "  ✅ $*"; pass=$((pass+1)); }
bad()  { echo "  ❌ $*"; fail=$((fail+1)); }
hdr()  { echo; echo "── $* ──"; }
note() { echo "  ·  $*"; }

# ── 每个判据返回值：0=绿 1=红（不碰全局计数，负控要复用）──────────────────
# C1：目录诊断集合 == 逐文件并集
chk_c1() {
    local bin="$1" dir="$2"
    "$bin" "$dir" > "$TMP/c1_dir.out" 2> "$TMP/c1_dir.err"
    head -n -1 "$TMP/c1_dir.out" | sort > "$TMP/c1_dir.set"
    : > "$TMP/c1_union.set"
    local f
    while IFS= read -r f; do
        "$bin" "$f" > "$TMP/c1_one.out" 2>/dev/null || true
        head -n -1 "$TMP/c1_one.out" >> "$TMP/c1_union.set"
    done < <(find "$dir" -name '*.px' -type f ! -path '*/.git/*' | sort)
    sort -o "$TMP/c1_union.set" "$TMP/c1_union.set"
    if [ ! -s "$TMP/c1_dir.set" ]; then
        note "目录模式输出为空（样本前提不成立，无法判并集）"
        return 1
    fi
    if ! diff -u "$TMP/c1_union.set" "$TMP/c1_dir.set" > "$TMP/c1.diff"; then
        note "并集不一致（左=逐文件，右=目录模式）：$(head -6 "$TMP/c1.diff" | tr '\n' ' ')"
        return 1
    fi
    note "并集 $(wc -l < "$TMP/c1_dir.set") 行逐行一致"
    return 0
}
# C2：rc 与「有没有错误」一致
chk_c2() {
    local bin="$1" dir="$2"
    "$bin" "$dir" > /dev/null 2>&1; local drc=$?
    "$bin" "$dir/clean_only" > /dev/null 2>&1; local crc=$?
    if [ "$drc" = "0" ]; then note "含错目录 rc=0（假绿）"; return 1; fi
    if [ "$crc" != "0" ]; then note "干净目录 rc=$crc（应为 0）"; return 1; fi
    note "含错目录 rc=$drc · 干净目录 rc=$crc"
    return 0
}
# C3：空目标 ≠ 干净
chk_c3() {
    local bin="$1" dir="$2"
    "$bin" "$dir" > "$TMP/c3.out" 2> "$TMP/c3.err"; local rc=$?
    if [ "$rc" = "0" ]; then note "空目录 rc=0（与干净不可区分）"; return 1; fi
    if ! grep -q '未找到 .px 文件' "$TMP/c3.err"; then note "空目录未出声（stderr 无「未找到」）"; return 1; fi
    note "空目录 rc=$rc 且 stderr 出声"
    return 0
}
# C4：单文件输出逐字节不变
chk_c4() {
    local bin="$1" dir="$2"
    local want="$TMP/c4.want" got="$TMP/c4.got"
    {
        echo "$dir/bad1.px:1:9: E L002: 未定义变量: 'undefined_thing_xyz'"
        echo "$dir/bad1.px: 1 错误, 0 警告"
    } > "$want"
    "$bin" "$dir/bad1.px" > "$got" 2>&1
    if ! diff -q "$want" "$got" > /dev/null; then
        note "单文件输出变了：$(diff "$want" "$got" | head -4 | tr '\n' ' ')"
        return 1
    fi
    note "单文件（含诊断 + 汇总行）逐字节一致"
    return 0
}
# C5：--json 与文本条数一致、file 是全路径
chk_c5() {
    local bin="$1" dir="$2"
    "$bin" "$dir" --json > "$TMP/c5.json" 2>/dev/null || true
    "$bin" "$dir" > "$TMP/c5.txt" 2>/dev/null || true
    local jn tn
    jn=$(grep -o '"file"' "$TMP/c5.json" | wc -l)
    tn=$(head -n -1 "$TMP/c5.txt" | grep -c . || true)
    if [ "$jn" != "$tn" ]; then note "json 条数 $jn != 文本条数 $tn"; return 1; fi
    if [ "$jn" = "0" ]; then note "json 条数为 0（样本前提不成立）"; return 1; fi
    if ! grep -q "\"file\":\"$dir/" "$TMP/c5.json"; then note "json 的 file 不是全路径"; return 1; fi
    note "json $jn 条 == 文本 $tn 条 · file 为全路径"
    return 0
}
# C6：.git 跳过 + 隐藏目录照扫
chk_c6() {
    local bin="$1" dir="$2"
    "$bin" "$dir" > "$TMP/c6.out" 2>/dev/null || true
    if grep -q 'should_skip_git_zzz' "$TMP/c6.out"; then note ".git 内部被扫（应跳过）"; return 1; fi
    if ! grep -q 'hidden_undefined_yyy' "$TMP/c6.out"; then note "隐藏目录未被扫（假绿温床）"; return 1; fi
    note ".git 跳过 ✅ · .hidden 照扫 ✅"
    return 0
}
# C7：悬空链接计错误且出声
chk_c7() {
    local bin="$1" dir="$2"
    "$bin" "$dir" > "$TMP/c7.out" 2> "$TMP/c7.err"; local rc=$?
    if [ "$rc" = "0" ]; then note "含悬空链接 rc=0（静默跳过）"; return 1; fi
    if ! grep -q '无法读取条目' "$TMP/c7.err"; then note "悬空链接未出声"; return 1; fi
    note "悬空链接 rc=$rc 且 stderr 点名"
    return 0
}
# C8：--strict 对警告生效
chk_c8() {
    local bin="$1" dir="$2"
    "$bin" "$dir" > /dev/null 2>&1; local nrc=$?
    "$bin" "$dir" --strict > /dev/null 2>&1; local src=$?
    if [ "$nrc" != "0" ]; then note "仅警告目录非 strict rc=$nrc（应为 0）"; return 1; fi
    if [ "$src" != "1" ]; then note "仅警告目录 strict rc=$src（应为 1）"; return 1; fi
    note "仅警告：非 strict rc=$nrc · strict rc=$src"
    return 0
}
# C9：硬词法错误不许被吞
chk_c9() {
    local bin="$1" dir="$2"
    "$bin" "$dir" > "$TMP/c9.out" 2> "$TMP/c9.err"; local rc=$?
    if [ "$rc" = "0" ]; then note "硬词法错误 rc=0（被吞）"; return 1; fi
    note "硬词法错误 rc=$rc（不静默）"
    return 0
}
# C10：性能上界
chk_c10() {
    local bin="$1" dir="$2"
    local t0 t1 dt
    t0=$(date +%s)
    "$bin" "$dir" > /dev/null 2>&1 || true
    t1=$(date +%s); dt=$((t1 - t0))
    if [ "$dt" -gt 60 ]; then note "100 文件耗时 ${dt}s > 60s"; return 1; fi
    note "100 文件耗时 ${dt}s（上界 60s）"
    return 0
}

# ── 环境准备：现场造样本（不往仓库里塞 .px —— 那些会进「发射冻结集」，
#    为 7 个 lint 语料付一次 freeze 重定基是拿判据的公信力换方便）──────────────
hdr "环境准备（现场造 fixtures，再叠 .git / 悬空链接 / 硬错 / 大量文件）"
D="$TMP/fx"
mkfixtures() {   # $1 = 根目录
    mkdir -p "$1/sub" "$1/.hidden" "$1/clean_only" "$1/empty"
    printf 'var a = undefined_thing_xyz\n' > "$1/bad1.px"
    printf 'var b = another_undefined_abc\n' > "$1/bad2.px"
    printf 'var c = 1\n' > "$1/clean.px"
    printf 'var d = 1 \n' > "$1/warn.px"
    printf 'var e = deep_undefined_zzz\n' > "$1/sub/deep_bad.px"
    printf 'var f = hidden_undefined_yyy\n' > "$1/.hidden/hidden_bad.px"
    printf 'var ok = 1\n' > "$1/clean_only/ok.px"
    : > "$1/empty/.keep"
}
mkfixtures "$D" || { echo "❌ 样本造不出来：$D"; exit 2; }
mkdir -p "$D/.git"
printf 'var g = should_skip_git_zzz\n' > "$D/.git/skip_me.px"
ln -s "$D/does_not_exist.px" "$D/dangling.px"
mkdir -p "$TMP/hard"
printf 'var h = 1\nvar bad = "\\*"\n' > "$TMP/hard/bad_escape.px"
note "样本目录 $D（含 .git / .hidden / sub / 悬空链接）"

# ── 前提自证：样本本身必须真的坏（否则 C1 两侧都空会假绿 · CRITERIA ④）──
hdr "前提自证（样本真坏、嵌套与隐藏真在、旧行为真错）"
"$ROOT/bootstrap/pxlint" "$D/bad1.px" > "$TMP/p_bad.out" 2>&1; PB=$?
if [ "$PB" != "0" ] && grep -q 'L002' "$TMP/p_bad.out"; then
    ok "P1 样本 bad1.px 单文件 rc=$PB 且报 L002（样本真坏）"
else
    bad "P1 样本 bad1.px 竟无 L002（性质变了，后续判据无意义）"
fi
[ -f "$D/sub/deep_bad.px" ] && ok "P2 嵌套样本在场" || bad "P2 嵌套样本缺失"
[ -f "$D/.hidden/hidden_bad.px" ] && ok "P3 隐藏目录样本在场" || bad "P3 隐藏目录样本缺失"
# P4：旧行为（bootstrap/pxlint）对目录必须假绿 —— 这是本条缺陷的现场
"$ROOT/bootstrap/pxlint" "$D" > "$TMP/p_dir.out" 2>&1; POD=$?
if [ "$POD" = "0" ]; then
    ok "P4 入库版 pxlint 对目录 rc=0（缺陷现场成立：这就是假绿）"
else
    note "P4 入库版对目录 rc=$POD（若已重烘为新版，此条转 SKIP —— 不算红）"
fi

# ── 构建被测二进制（从源码构建，测的是源码而不是入库件）─────────────────
hdr "构建被测 pxlint（tools/px build tools/pxlint.px）"
if ! "$PXBUILD" build tools/pxlint.px > "$TMP/build_main.log" 2>&1; then
    echo "❌ 构建失败："; tail -5 "$TMP/build_main.log"; exit 2
fi
BIN="$ROOT/tools/build/pxlint"
[ -x "$BIN" ] || { echo "❌ 未产出 $BIN"; exit 2; }
note "$("$BIN" --version 2>&1 | head -1) @ $BIN"

# ── 判据 C1–C10 ──────────────────────────────────────────────────────────
hdr "C1 目录诊断集合 == 逐文件并集"
if chk_c1 "$BIN" "$D"; then ok "C1 并集一致"; else bad "C1 并集不一致（目录模式漏报/多报）"; fi

hdr "C2 rc 与「有没有错误」一致"
if chk_c2 "$BIN" "$D"; then ok "C2 rc 语义正确"; else bad "C2 rc 语义错（假绿或假红）"; fi

hdr "C3 空目标 ≠ 干净"
if chk_c3 "$BIN" "$D/empty"; then ok "C3 空目录可区分"; else bad "C3 空目录与干净不可区分"; fi

hdr "C4 单文件输出逐字节不变"
if chk_c4 "$BIN" "$D"; then ok "C4 旧契约未变"; else bad "C4 单文件输出被改动"; fi

hdr "C5 --json 目录模式"
if chk_c5 "$BIN" "$D"; then ok "C5 json 一致且全路径"; else bad "C5 json 不一致"; fi

hdr "C6 .git 跳过 · 隐藏目录照扫"
if chk_c6 "$BIN" "$D"; then ok "C6 遍历口径正确"; else bad "C6 遍历口径错"; fi

hdr "C7 悬空链接计错误并出声"
if chk_c7 "$BIN" "$D"; then ok "C7 读不到不静默"; else bad "C7 读不到被静默跳过"; fi

hdr "C8 --strict 对目录警告生效"
WARNONLY="$TMP/warnonly"; mkdir -p "$WARNONLY"
printf 'var d = 1 \n' > "$WARNONLY/warn.px"
if chk_c8 "$BIN" "$WARNONLY"; then ok "C8 strict 语义正确"; else bad "C8 strict 语义错"; fi

hdr "C9 硬词法错误不许被吞"
if chk_c9 "$BIN" "$TMP/hard"; then ok "C9 硬错不静默"; else bad "C9 硬错被吞成 0 错误"; fi

hdr "C10 性能上界（100 文件 < 60s）"
PERF="$TMP/perf"; mkdir -p "$PERF"
i=0
while [ "$i" -lt 100 ]; do
    printf 'var p%s = 1\n' "$i" > "$PERF/f$i.px"
    i=$((i+1))
done
if chk_c10 "$BIN" "$PERF"; then ok "C10 性能在上界内"; else bad "C10 性能越界"; fi

# ── 负控 A–D ─────────────────────────────────────────────────────────────
neg() {   # neg <名> <打桩函数> <判据函数> <判据参数...>
    local name="$1" pfn="$2" cfn="$3"; shift 3
    hdr "负控 $name"
    local src="pxlint_neg$name.px"
    cp "$ROOT/tools/pxlint.px" "$ROOT/tools/$src"
    NEG_SRC+=("$src")
    if ! "$pfn" "$ROOT/tools/$src"; then bad "负控 $name：打桩失败（锚点没找到）"; return 1; fi
    if ! "$PXBUILD" build "tools/$src" > "$TMP/build_neg$name.log" 2>&1; then
        bad "负控 $name：负控副本编译失败"; tail -3 "$TMP/build_neg$name.log"; return 1
    fi
    local nbin="$ROOT/tools/build/pxlint_neg$name"
    if [ ! -x "$nbin" ]; then bad "负控 $name：未产出 $nbin"; return 1; fi
    if "$cfn" "$nbin" "$@"; then
        bad "负控 $name：判据仍为绿（负控没有牙）"
    else
        echo "   ✅ 负控 $name：判据已判红（符合预期）"
    fi
    rm -f "$ROOT/tools/$src" "$nbin"
}
patch_A() {   # 只跑第一个文件
    python3 - "$1" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
a = "        let diags = r[0]\n        nfiles += 1\n"
assert a in s, 'anchor A missing'
s = s.replace(a, a + "        fi = len(files)   # M387 负控 A：只跑第一个文件\n", 1)
open(p, 'w', encoding='utf-8').write(s)
PY
}
patch_B() {   # 删掉 is_dir 分支（回到「目录当文件」）
    python3 - "$1" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
a = ("    # M387（缺陷 700）：目录模式 —— 先判类型再决定路径，绝不再把目录当文件读\n"
     "    if is_dir(file):\n        lx_run_dir(file, jmode, strict)\n")
assert a in s, 'anchor B missing'
s = s.replace(a, "", 1)
open(p, 'w', encoding='utf-8').write(s)
PY
}
patch_C() {   # 空目录当干净
    python3 - "$1" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
a = ('    if nfiles == 0:\n        print_err(dir + ": 未找到 .px 文件（空目标不算干净）")\n        exit(1)\n')
assert a in s, 'anchor C missing'
s = s.replace(a, '    if nfiles == 0:\n        exit(0)   # M387 负控 C：空目标当干净\n', 1)
open(p, 'w', encoding='utf-8').write(s)
PY
}
patch_D() {   # 静默跳过悬空链接
    python3 - "$1" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
a = ('            if st == null:\n'
     '                ferrs.append("pxlint: 无法读取条目 \'" + full + "\'（stat 返回 null，悬空链接或权限不足）")\n'
     '                continue\n')
assert a in s, 'anchor D missing'
s = s.replace(a, '            if st == null:\n                continue   # M387 负控 D：静默跳过\n', 1)
open(p, 'w', encoding='utf-8').write(s)
PY
}

neg A patch_A chk_c1 "$D"
neg B patch_B chk_c1 "$D"
neg C patch_C chk_c3 "$D/empty"
neg D patch_D chk_c7 "$D"

# ── 收尾：负控副本必须清干净（不许留残件被烘进入库件）──────────────────
hdr "收尾自证"
LEFT=$(ls "$ROOT"/tools/pxlint_neg*.px 2>/dev/null | wc -l)
if [ "$LEFT" = "0" ]; then ok "负控副本已清空"; else bad "残留 $LEFT 个 tools/pxlint_neg*.px"; fi
# 判据贴「门自己有没有改源」，不贴「工作树干不干净」—— 开发中源本来就没提交，
# 拿 git status 判会把「我这轮正在改它」误判成「门没还原」（CRITERIA ⑨：判据找不到先怀疑判据）。
SHA1=$(sha256sum "$ROOT/tools/pxlint.px" | cut -d' ' -f1)
if [ "$SHA1" = "$SHA0" ]; then
    ok "tools/pxlint.px 逐字节未被门改动"
else
    bad "tools/pxlint.px 被门改动（负控未还原）"
fi

echo
echo "  合计：绿 $pass · 红 $fail"
if [ "$fail" = 0 ]; then
    echo "M387-VERIFY-OK"
    exit 0
else
    echo "M387-VERIFY-FAILED"
    exit 1
fi
