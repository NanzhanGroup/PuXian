#!/usr/bin/env bash
# ============================================================
# M184 门（第 62 轮）：字符串→数值「严格解析」+ `mkdir` 返回 bool + `list_dir_opt`
# ------------------------------------------------------------
# 三条缺陷全部来自第三方 `banshanhanfu/registry-px` 的登记（写库过程中暴露）：
#
#  ① DEF-002 **`int()`/`float()` 是宽容前缀解析**（C atoi 家族）：
#      `int("12ab")`=12 · `int("abc")`=0 · `int("1e3")`=1 · `int("0x10")`=0
#      · `float("1.2.3")`=1.2 · `float("x")`=0.0  ⇒ **静默错值**；
#      且 `int("")` **三轨分叉**（解释轨报 R1002、编译轨给 0）。
#     修后：trim（space/\t/\n/\r 四种，与既有 `trim()` 逐字符同集）后必须**整体合法**
#      ⇒ `R1002 无法将 '<原文>' 转为 int/float`（对齐 Python int()/float()、Go strconv）。
#      · int 仅十进制 `[+-]?[0-9]+`（拒 1e3 / 0x10 / 1.5 / 超 int64 范围）
#      · float 十进制 + 指数，另有 `inf/infinity/nan` 白名单（可选符号、大小写不敏感）；
#        `strtod` 会吃的 `0x10` 十六进制浮点被**显式拒绝**。
#      ⚠️ 空白集必须与 `bi_trim` 相等（`\f`/`\v` 不在内）—— 否则 `int("\f12")`
#        解释轨拒、编译轨收（三轨分叉）。这条是本门第 ③ 层"边界一致性"的由来。
#
#  ② DEF-005 **`mkdir()` 成功返回 `null`** ⇒ `if mkdir(d):` **永远为假**（把成功误判成失败），
#      且逐层 mkdir 返回值被丢弃 ⇒ "创建失败"完全无声。
#     修后对齐 Go `os.MkdirAll`：成功（含"已存在且确实是目录"）⇒ `true`，失败 ⇒ `false`；
#      同名**非目录**（普通文件）⇒ `false`（否则 `mkdir("/etc/passwd")` 会假真）。
#
#  ③ DEF-014 **`list_dir` 无安全变体**：不存在的目录/非目录直接 `px_error` **终止进程**，
#      与 `json_parse`→`json_parse_opt`、`read_file`→`read_file_opt` 的"安全变体把 err
#      通道显式化"先例不一致（glob / 递归遍历类库首当其冲）。
#     修后新增 `list_dir_opt(path) → Ok(list) | Err(msg)`；`list_dir` **保持**终止语义。
#
# 判据（四层正判据 + 三道负控，每层/每道可独立判红）：
#  ① 合法侧 `num_ok.px`（22 断言）三轨 rc=0 且输出**逐字节一致**；
#  ② 拒绝侧 `num_neg/*.px`（**27 用例 × 三轨**）rc≠0 且三轨提取出的词条**逐字相同**
#     （位置前缀允许不同 —— 已登记缺陷 186 的既有通道差异：解释轨写出 `行:列`，
#      编译轨写 `[函数 行N]`）；
#  ③ `mkdir_sem.px` 六条语义断言三轨一致；
#  ④ `listdir_sem.px` 五条语义断言三轨一致 + `before` 必须先打印（证明"没被终止"是流程性的）。
#
# 负控（各自独立判红，源逐字节还原）：
#  A px_str_to_i64 退化为 `atoll` 直通 ⇒ ② 必须红（编译轨不再拒绝非法串）
#  B bi_mkdir 退回 `return px_null()`   ⇒ ③ 必须红
#  C 删掉 `list_dir_opt` 注册            ⇒ ④ 必须红
#
# 用法：bash examples/m184_num_fs_strict/verify.sh [--neg-skip]
# 退出码：0 = 绿，1 = 红。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT"
export LC_ALL=C LANG=C

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

SRC_RT="runtime/runtime.c"
WORK="$(mktemp -d /tmp/m184.XXXXXX)"
fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

snapshot()    { cp "$SRC_RT" "$WORK/runtime.c.bak"; }
restore_all() { [ -f "$WORK/runtime.c.bak" ] && cp "$WORK/runtime.c.bak" "$SRC_RT"; }
trap 'restore_all; rm -rf "$WORK"' EXIT
# 每道负控前先 restore_all 再 snapshot（各自独立、干净起点 —— M165/M181 教训）
snapshot
sha() { sha256sum "$SRC_RT" | cut -c1-16; }
SHA0="$(sha)"

# ─────────── 三轨 runner ───────────
# run3 <源文件绝对路径> <tag> → 产出 $WORK/<tag>.{interp,vm,c}.{out,rc}
run3() {
    # ⚠️ `local a=$1 b=$2 c=$WORK/b$b` 会踩 set -u：bash 对**整条简单命令**先做 word
    #   展开再执行内建 `local` ⇒ `$b` 尚未赋值 ⇒ `unbound variable`。必须**拆行**。
    local src="$1" tag="$2"
    local d="$WORK/b$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$src" "$d/"
    local b; b="$(basename "$src" .px)"
    rm -rf "$d/build"
    # 解释轨（⚠️ 必须用 $ROOT 绝对路径：下面 cd 进了 $d）
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$b.px" ) > "$WORK/$tag.interp.out" 2>&1
    echo $? > "$WORK/$tag.interp.rc"
    # VM 轨（用户面默认）
    timeout 300 ./tools/px build "$d/$b.px" > "$WORK/$tag.vmbuild.log" 2>&1
    if [ -x "$d/build/$b" ]; then
        timeout 60 "$d/build/$b" > "$WORK/$tag.vm.out" 2>&1
        echo $? > "$WORK/$tag.vm.rc"
    else
        echo 999 > "$WORK/$tag.vm.rc"; tail -3 "$WORK/$tag.vmbuild.log" > "$WORK/$tag.vm.out"
    fi
    rm -rf "$d/build"
    # C 轨（逃生舱）
    PX_BUILD_ENGINE=c timeout 300 ./tools/px build "$d/$b.px" > "$WORK/$tag.cbuild.log" 2>&1
    if [ -x "$d/build/$b" ]; then
        timeout 60 "$d/build/$b" > "$WORK/$tag.c.out" 2>&1
        echo $? > "$WORK/$tag.c.rc"
    else
        echo 999 > "$WORK/$tag.c.rc"; tail -3 "$WORK/$tag.cbuild.log" > "$WORK/$tag.c.out"
    fi
}

tri_same() {   # <tag>：三轨 stdout 逐字节一致
    cmp -s "$WORK/$1.interp.out" "$WORK/$1.vm.out" || return 1
    cmp -s "$WORK/$1.interp.out" "$WORK/$1.c.out"  || return 1
    return 0
}
tri_rc0() {    # <tag>：三轨 rc=0
    [ "$(cat "$WORK/$1.interp.rc")" = 0 ] || return 1
    [ "$(cat "$WORK/$1.vm.rc")" = 0 ]     || return 1
    [ "$(cat "$WORK/$1.c.rc")" = 0 ]      || return 1
    return 0
}
has() { grep -q "$2" "$WORK/$1" ; }

# 从三轨输出里提取统一词条（`无法将 '…' 转为 int|float`）——位置前缀允许不同
verbatim() { grep -o "无法将 '[^']*' 转为 \(int\|float\)" "$1" | head -1; }

# ─────────── 判据 ───────────
chk_num_ok() {      # ① 合法侧：三轨一致 + rc=0 + 22 行
    run3 "$HERE/num_ok.px" numok || return 1
    tri_rc0 numok || return 1
    tri_same numok || return 1
    [ "$(wc -l < "$WORK/numok.interp.out")" -ge 22 ] || return 1
    has numok.interp.out "^i5=9223372036854775807$" || return 1
    has numok.interp.out "^f10=nan$" || return 1
    has numok.interp.out "^i9=123$" || return 1        # 前导零（首版回归的钉子）
    return 0
}
chk_num_neg() {     # ② 拒绝侧：27 用例 × 三轨：rc≠0 + R1002 + 词条逐字相同
    local f b rc tag ok=0 n=0
    for f in "$HERE"/num_neg/*.px; do
        b="$(basename "$f" .px)"
        n=$((n + 1))
        run3 "$f" "neg_$b" || return 1
        for tag in interp vm c; do
            rc="$(cat "$WORK/neg_$b.$tag.rc")"
            [ "$rc" != 0 ] || { note "拒绝侧未拒绝($tag): $b rc=$rc"; return 1; }
            grep -q "R1002" "$WORK/neg_$b.$tag.out" || { note "缺 R1002($tag): $b"; return 1; }
        done
        local v1 v2 v3
        v1="$(verbatim "$WORK/neg_$b.interp.out")"
        v2="$(verbatim "$WORK/neg_$b.vm.out")"
        v3="$(verbatim "$WORK/neg_$b.c.out")"
        [ -n "$v1" ] || { note "词条提取失败(interp): $b"; return 1; }
        [ "$v1" = "$v2" ] && [ "$v1" = "$v3" ] || { note "词条不一致: $b [$v1][$v2][$v3]"; return 1; }
        has neg_$b.interp.out "^before$" || { note "未执行到转换点: $b"; return 1; }
        ok=$((ok + 1))
    done
    [ "$ok" = 27 ] && [ "$n" = 27 ] || { note "用例数不符: $n"; return 1; }
    return 0
}
chk_mkdir() {       # ③ mkdir 语义（六条）
    rm -rf /tmp/m184_gate_fs
    run3 "$HERE/mkdir_sem.px" mkdirsem || return 1
    tri_rc0 mkdirsem || return 1
    tri_same mkdirsem || return 1
    has mkdirsem.interp.out "^fresh=true type=bool$" || return 1
    has mkdirsem.interp.out "^exists=true$"          || return 1
    has mkdirsem.interp.out "^nested=true$"          || return 1
    has mkdirsem.interp.out "^IF_MKDIR_TRUE$"        || return 1
    has mkdirsem.interp.out "^notdir=false$"         || return 1
    has mkdirsem.interp.out "^is_dir_n3=true$"       || return 1
    return 0
}
chk_listdir() {     # ④ list_dir_opt 语义（五条 + before）
    rm -rf /tmp/m184_gate_fs; mkdir -p /tmp/m184_gate_fs
    run3 "$HERE/listdir_sem.px" ldsem || return 1
    tri_rc0 ldsem || return 1
    tri_same ldsem || return 1
    has ldsem.interp.out "^before$"              || return 1
    has ldsem.interp.out "^missing_is_err=true$" || return 1
    has ldsem.interp.out "Err(fs: 读取目录失败"  || return 1
    has ldsem.interp.out "os error"              || return 1
    has ldsem.interp.out "^exist_is_ok=true$"    || return 1
    has ldsem.interp.out "^notdir_is_err=true$"  || return 1
    has ldsem.interp.out "^same_as_list_dir=true$" || return 1
    return 0
}

hdr "① 合法侧：int/float 严格解析的接受面（22 断言 · 三轨逐字节一致）"
chk_num_ok && echo "   ✅ num_ok：三轨一致，接受面完整" || bad "① num_ok 判据失败"
hdr "② 拒绝侧：27 用例 × 三轨（rc≠0 + R1002 + 词条逐字相同）"
chk_num_neg && echo "   ✅ num_neg：27 用例三轨全部响亮拒绝，词条一致" || bad "② num_neg 判据失败"
hdr "③ mkdir 返回值语义（null → bool，对齐 Go os.MkdirAll）"
chk_mkdir && echo "   ✅ mkdir_sem：六条语义断言三轨一致" || bad "③ mkdir_sem 判据失败"
hdr "④ list_dir_opt 安全变体（不再终止进程）"
chk_listdir && echo "   ✅ listdir_sem：五条语义断言三轨一致 + before 打印" || bad "④ listdir_sem 判据失败"

# ─────────── 负控 ───────────
neg() {   # neg <名> <判据函数> <打桩函数>
    local name="$1" fn="$2" patcher="$3"
    restore_all; snapshot
    # ⚠️ 不用 eval：打桩一律写成**具名函数**（含 heredoc 的字符串过 eval 易踩解析坑）。
    # ⚠️ `hdr` / `echo` 的字符串里**绝不写反引号** —— bash 会对双引号内的反引号做
    #    **命令替换**。M184 实测：`hdr "负控 A：\`px_str_to_i64\` …"` 当场去执行
    #    px_str_to_i64 ⇒ `command not found`，而负控"看起来"在跑。
    #    （同型 bug 本轮在 `selfhost/rebake_bin.sh` 也修过一次 —— 凡给人看的字符串，不用反引号。）
    $patcher || { bad "负控 $name：打桩失败"; return 1; }
    if [ "$(sha)" = "$SHA0" ]; then bad "负控 $name：打桩未生效（源未变）"; return 1; fi
    if $fn; then
        bad "负控 $name：判据仍为绿（负控没有牙）"
    else
        echo "   ✅ 负控 $name：判据已判红（符合预期）"
    fi
    restore_all
    [ "$(sha)" = "$SHA0" ] || bad "负控 $name：源未逐字节还原"
}

# ── 负控打桩（各自独立、干净起点）──
patch_A() {   # int 解析退回 atoll 直通
    python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
anchor = 'static int px_str_to_i64(const char* s, int len, int64_t* out) {'
assert anchor in s, 'anchor missing'
s = s.replace(anchor, anchor + '\n    *out = (int64_t)atoll(s); return 1;   /* M184 负控 A */', 1)
open(p, 'w', encoding='utf-8').write(s)
PY
}
patch_B() {   # mkdir 退回返回 null
    python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
anchor = '    if (ok && (stat(path, &st) != 0 || !S_ISDIR(st.st_mode))) ok = 0;\n    return px_bool(ok);'
assert anchor in s, 'anchor missing'
s = s.replace(anchor, '    (void)ok;\n    return px_null();   /* M184 负控 B */', 1)
open(p, 'w', encoding='utf-8').write(s)
PY
}
patch_C() {   # 删掉 list_dir_opt 注册行（按 marker 定位，避开中文注释匹配）
    python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
marker = 'px_set_global("list_dir_opt"'
i = s.index(marker)
j = s.index('\n', i)
open(p, 'w', encoding='utf-8').write(s[:i] + s[j + 1:])
PY
}

if [ "$NEG_SKIP" = 1 ]; then
    hdr "负控：--neg-skip（CI 用）"
    note "跳过三道负控（本地/里程碑收尾必须全跑）"
else
    hdr "负控 A：px_str_to_i64 退化为 atoll 直通 ⇒ ② 必须红"
    neg A chk_num_neg patch_A

    hdr "负控 B：bi_mkdir 退回 return px_null() ⇒ ③ 必须红"
    neg B chk_mkdir patch_B

    hdr "负控 C：删掉 list_dir_opt 注册 ⇒ ④ 必须红"
    neg C chk_listdir patch_C
fi

echo
if [ "$fail" = 0 ]; then
    echo "M184-VERIFY-OK"
else
    echo "M184-VERIFY-FAILED"
fi
exit $fail
