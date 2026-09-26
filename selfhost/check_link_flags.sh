#!/usr/bin/env bash
# ============================================================
# check_link_flags.sh —— 构建脚本的**链接 flag 卫生**守卫（M219）
#
# 为什么需要它（缺陷 311/312 的根因面）：
#   本仓预置的三方资产（`sqlite3.o` / `libz.a` / `mbedtls/*.a`）是**非 PIC** 对象。
#   任何「默认 PIE」的宿主工具链（Ubuntu/Debian gcc 是 `--enable-default-pie`）把它们
#   链进 PIE 可执行文件都会报
#       relocation R_X86_64_32[S] against `.rodata' can not be used when making a PIE object
#   而 Red Hat 系 gcc **默认非 PIE** ⇒ 同一份脚本在开发机恒绿、在 CI 必红。
#   ⇒ 判据「**链接必须显式声明 PIE 口径**」：`-static`（本仓口径）或 `-no-pie`/`-pie`。
#
# 用法：
#   bash selfhost/check_link_flags.sh [仓库根]     # 扫描（默认 .）
#   bash selfhost/check_link_flags.sh --self-test  # 自证（fixture 期望违例集**精确相等** + 1 道负控）
#   bash selfhost/check_link_flags.sh --list       # 列出扫描面与豁免表
#
# 判据：
#   ① 只查「**链接**命令」：有编译器 token + `-o` 整词 + **无** `-c/-E/-S`
#   ② 跳过：注释行 · 消息行（echo/printf/note/bad/info/warn/die）· **heredoc 体** · 判据自身
#   ③ 合规：出现 `-static`/`-static-pie`/`-no-pie`/`-pie`/`-shared`
#   ④ 豁免两条路（都**必须带理由**且**计数**，不静默）：
#      ④a 变量（`$st` 等口径参数化）
#      ④b **行级**（`path:line`）—— 且必须**仍然是一条被扫到的链接命令**，
#          否则判红「豁免失效（行号漂移/该行已不是链接）」⇒ 豁免表不会悄悄过期
#   ⑤ 续行（行尾 `\`）拼接后再判 —— 否则「flag 在上一行」会被漏判
# ============================================================
set -u

ROOT="."
SELF_TEST=0
LIST_ONLY=0

# ---- 扫描面（**显式声明**，不靠 find 猜）----
SCAN_GLOBS=(
    'selfhost/*.sh'
    'tools/px'
    'tools/*.sh'
    'packaging/*.sh'
    'examples/*/verify.sh'
    'examples/*/*.sh'
    'examples/*/*/*.sh'
)

# ---- 豁免：变量（口径参数化，值由调用方决定）----
EXEMPT_VARS=( '$st' '$FLAGS' '$pie_flag' '$fl' )
EXEMPT_VARS_WHY=( 'rebake_bin.sh：static|dynamic 两档参数化（M114-S2 有意）'
                  'linkit.sh：MODE 参数化，默认 static'
                  'tools/px：riscv64 交叉档显式 -no-pie（与 -static 同行）'
                  'M219 门：探针的 flag 段由 extract_flags **从被测脚本源码抽出**（抽出的串就是被测对象；$fl2 是故意去掉 -static 的负例，必须失败）' )

# ---- 豁免：行级（**必须窄**，逐条带理由；失效即判红）----
EXEMPT_LINES=(
    'selfhost/check_bin_portability.sh:140|动态对照件——该门**故意**产一个动态件，用来证明可移植性判据有区分力'
    'selfhost/native_bootstrap.sh:202|特性探测（只问 -ldl/-lpthread 在不在），源文件无三方非 PIC 资产 ⇒ 与 PIE 无关'
    'selfhost/native_bootstrap.sh:215|静态 flag 由 `_link_pxc0` 的 `$1`（= $LINK_EXTRA）传入；动态回退档有**响亮警告 + 全静态硬门**兜底'
)

# ---- 合规正则（**单点**定义 ⇒ 负控「判据自伤」只能改这一处）----
COMPLIANT_RX='(^|[[:space:]])(-static|-static-pie|-no-pie|-pie|-shared)([[:space:]]|$)'
CC_TOKENS='gcc|cc|clang|\$cc|\$\{cc\}|\$CC|\$\{CC\}'
# 本文件是**判据本身** ⇒ 显式跳过（其内 fixture 文本会自触发，见 M213 先例）
SELF_PATH_RE='check_link_flags\.sh'

n_files=0 n_links=0 n_viol=0 n_exempt_var=0 n_exempt_line=0 n_skip_self=0
VIOL=() INFO=()
declare -A SEEN_LINK=()
declare -A SEEN_FILE=()

_strip_lead() { printf '%s' "${1#"${1%%[![:space:]]*}"}"; }

# 取**命令位置**的首个 token（剥掉控制关键字与环境赋值前缀）⇒ 全局 `_TOK`
#   为什么需要：`if grep -q '^ gcc -O2 -o x' f` / `sed -i 's|gcc -O2 -o x|…|'` 这类行
#   里含有「编译器 token + -o」的**字符串**，但它**不是**一条链接命令
#   ⇒ 必须按「谁在跑」判，不能按「行里有没有那个词」判（M213「首版把自己判红」的同类）。
#   ⚠️ 用**全局变量**传出（不再 `$( )`）——本函数每行都要调，266 文件实测：
#      命令替换版 1m19s → 全局版 ~4s（fork 数从 ~4 万降到 ~百）。
_TOK=""
_set_first_cmd_token() {
    local s="$1" t
    _TOK=""
    while :; do
        s="${s#"${s%%[![:space:]]*}"}"
        [ -n "$s" ] || return 1
        if [[ "$s" == *[[:space:]]* ]]; then
            t="${s%%[[:space:]]*}"; s="${s#*[[:space:]]}"
        else
            t="$s"; s=""
        fi
        case "$t" in
            if|then|else|elif|do|while|until|'!'|'{'|'|') continue ;;
            *=*)                                       continue ;;   # 环境/变量赋值前缀
            *) _TOK="$t"; return 0 ;;
        esac
    done
}

# 纯 bash 快速否决：绝大多数行不含编译器名 ⇒ 一次 case 就退出（否则每行两次 fork）
_prefilter() {
    case "$1" in
        *gcc*|*clang*|*'$cc'*|*'${cc}'*|*'$CC'*|*'${CC}'*) return 0 ;;
        *) return 1 ;;
    esac
}

check_logical() {   # $1=显示名 $2=起始行 $3=拼好的逻辑行
    local f="$1" ln="$2" cmd="$3" lead
    lead="${cmd#"${cmd%%[![:space:]]*}"}"
    case "$lead" in \#*) return 0 ;; esac
    # ② 消息行 / **模式行**（grep·sed·awk… 里带着链接文本，但不是链接）⇒ 跳过
    _set_first_cmd_token "$cmd" || return 0
    case "$_TOK" in
        echo|printf|note|bad|ok|info|warn|warnf|die|say) return 0 ;;
        grep|egrep|fgrep|sed|awk|case|test|'['|'[[')     return 0 ;;
    esac
    _prefilter "$cmd" || return 0
    printf '%s' "$cmd" | grep -qE "(^|[[:space:]\"'])(${CC_TOKENS})([[:space:]\"']|$)" || return 0
    printf '%s' "$cmd" | grep -qw -- '-o' || return 0
    printf '%s' "$cmd" | grep -qwE -- '-(c|E|S)' && return 0
    n_links=$((n_links+1)); SEEN_LINK["$f:$ln"]=1
    printf '%s' "$cmd" | grep -qE "$COMPLIANT_RX" && return 0
    local i v
    for i in "${!EXEMPT_VARS[@]}"; do
        v="${EXEMPT_VARS[$i]}"
        case "$cmd" in *"$v"*) n_exempt_var=$((n_exempt_var+1))
            INFO+=("$f:$ln|变量豁免（$v）—— ${EXEMPT_VARS_WHY[$i]}"); return 0 ;; esac
    done
    local e
    for e in "${EXEMPT_LINES[@]}"; do
        case "$f:$ln" in "${e%%|*}") n_exempt_line=$((n_exempt_line+1))
            INFO+=("$f:$ln|行级豁免—— ${e#*|}"); return 0 ;; esac
    done
    n_viol=$((n_viol+1))
    VIOL+=("$f:$ln|$(_strip_lead "$cmd" | cut -c1-108)")
}

scan_file() {   # $1=可读路径 $2=显示名
    local f="$1" disp="$2" line lineno=0 start=0 buf="" hd=""
    case "$disp" in *"$SELF_PATH_RE"*) n_skip_self=$((n_skip_self+1))
        INFO+=("$disp|-|判据自身（显式跳过）"); return 0 ;; esac
    n_files=$((n_files+1))
    while IFS= read -r line || [ -n "$line" ]; do
        lineno=$((lineno+1))
        if [ -n "$hd" ]; then                        # ② heredoc 体内 ⇒ 跳过
            [ "$line" = "$hd" ] && hd=""
            continue
        fi
        case "$line" in *'<<'*)
            if [[ "$line" =~ \<\<-?[\'\"]?([A-Za-z_][A-Za-z0-9_]*)[\'\"]? ]]; then
                hd="${BASH_REMATCH[1]}"
            fi ;;
        esac
        [ -n "$buf" ] || start=$lineno
        case "$line" in                              # ⑤ 续行拼接
            *\\) buf+="${line%\\} "; continue ;;
        esac
        buf+="$line"
        check_logical "$disp" "$start" "$buf"
        buf=""
    done < "$f"
    [ -n "$buf" ] && check_logical "$disp" "$start" "$buf"
    return 0
}

run_scan() {
    local g f disp
    shopt -s nullglob
    for g in "${SCAN_GLOBS[@]}"; do
        for f in $ROOT/$g; do
            [ -f "$f" ] || continue
            disp="${f#"$ROOT"/}"; [ "$disp" = "$f" ] && disp="${f#./}"
            [ -n "${SEEN_FILE[$disp]:-}" ] && continue      # 多组 glob 命中同一文件 ⇒ 只扫一次
            SEEN_FILE[$disp]=1
            scan_file "$f" "$disp"
        done
    done
    shopt -u nullglob
}

# 豁免表**过期判据**：每条行级豁免必须仍命中一条被扫到的链接命令
check_exempt_stale() {
    local e key
    for e in "${EXEMPT_LINES[@]}"; do
        key="${e%%|*}"
        [ -n "${SEEN_LINK[$key]:-}" ] && continue
        n_viol=$((n_viol+1))
        VIOL+=("[豁免失效] $key|豁免失效：该行不再是链接命令（行号漂移？已改？）—— 请更新 selfhost/check_link_flags.sh 的豁免表")
    done
}

report() {
    local x
    echo "── 扫描面：${#SCAN_GLOBS[@]} 组 glob · 命中文件 $n_files（跳过判据自身 $n_skip_self）"
    echo "── 链接命令 $n_links 条 · 合规 $((n_links-n_viol-n_exempt_var-n_exempt_line)) · 豁免 $((n_exempt_var+n_exempt_line))（变量 $n_exempt_var / 行级 $n_exempt_line）"
    for x in "${INFO[@]:-}"; do [ -n "$x" ] && echo "   ℹ️  ${x%%|*}  —— ${x#*|}"; done
    for x in "${VIOL[@]:-}"; do [ -n "$x" ] && echo "   ❌ ${x%%|*}  —— ${x#*|}"; done
    if [ "$n_viol" = 0 ]; then
        echo "   ✅ 链接 flag 卫生：违例 0 条（缺 -static/-no-pie 的链接会在默认 PIE 工具链上必红）"
        return 0
    fi
    echo "   ❌ 违例 $n_viol 条 —— 开发机（Red Hat 系，默认非 PIE）绿、CI（Ubuntu）红"
    echo "      ⇒ 修法：链接加 -static（本仓口径，见 selfhost/rebake_bin.sh 的 link_vm_track）或 -no-pie"
    return 1
}

# ---------------- 自证 ----------------
self_test() {
    local T ok=0 bad=0 got want save_root
    T="$(mktemp -d "${TMPDIR:-/tmp}/px-linkfl.XXXXXX")" || exit 2
    mkdir -p "$T/proj/selfhost" "$T/proj/examples/g1" "$T/proj/examples/g2" \
             "$T/proj/examples/g3" "$T/proj/tools" "$T/proj/packaging"
    printf '#!/usr/bin/env bash\ngcc -O2 -pthread -o "$out" a.o $objs\n'                      > "$T/proj/selfhost/bad.sh"
    printf '#!/usr/bin/env bash\ngcc -static -O2 -pthread -o "$out" a.o $objs\n'             > "$T/proj/selfhost/ok_static.sh"
    printf '#!/usr/bin/env bash\ngcc -no-pie -O2 -o "$out" a.o\n'                            > "$T/proj/selfhost/ok_nopie.sh"
    printf '#!/usr/bin/env bash\ngcc -c -O2 -o a.o a.c\n'                                    > "$T/proj/selfhost/skip_compile.sh"
    printf '#!/usr/bin/env bash\n# gcc -O2 -pthread -o "$out" a.o\n'                         > "$T/proj/selfhost/skip_comment.sh"
    printf '#!/usr/bin/env bash\necho "gcc -O2 -o out a.o"\n'                                > "$T/proj/selfhost/skip_echo.sh"
    printf '#!/usr/bin/env bash\ncat > x <<%sEOF%s\ngcc -O2 -pthread -o out a.o\nEOF\n' "'" "'" > "$T/proj/selfhost/skip_heredoc.sh"
    printf '#!/usr/bin/env bash\nls -o /tmp\n'                                               > "$T/proj/selfhost/skip_notcc.sh"
    printf '#!/usr/bin/env bash\ngcc -O2 -pthread -o "$out" \\\\\n    a.o $objs\n'           > "$T/proj/examples/g1/verify.sh"
    printf '#!/usr/bin/env bash\ngcc -O2 -pthread \\\\\n    -static -o "$out" a.o\n'         > "$T/proj/examples/g2/verify.sh"
    printf '#!/usr/bin/env bash\ngcc $st -O2 -pthread -o "$out" a.o\n'                       > "$T/proj/packaging/var_exempt.sh"
    printf '#!/usr/bin/env bash\ncc_cmd="$cc -static -O2 -o \\"$out\\" a.o"\n'               > "$T/proj/tools/px"
    # 行级豁免样例：由本文件**自己**的豁免表决定 ⇒ 用一条与表内 path:line 不匹配的行（不豁免）
    printf '#!/usr/bin/env bash\ngcc -O2 -pthread -o "$out" b.o\n'                           > "$T/proj/examples/g3/verify.sh"

    want="examples/g1/verify.sh examples/g3/verify.sh selfhost/bad.sh"
    # ⚠️ 必须**在当前 shell** 跑：计数器/违例表是**全局**，包进 `$( )` 会落进子 shell（首版实测全假红）
    save_root="$ROOT"; ROOT="$T/proj"; run_scan; ROOT="$save_root"
    got="$(printf '%s\n' "${VIOL[@]:-}" | cut -d'|' -f1 | sed 's/:[0-9]*$//' | sort -u | tr '\n' ' ' | sed 's/ *$//')"
    if [ "$got" = "$want" ]; then echo "   ✅ F1 期望违例集精确相等（[$want]）"; ok=$((ok+1))
    else echo "   ❌ F1 期望违例集 [$want]，实得 [$got]"; bad=$((bad+1)); fi

    if [ "$n_links" = 8 ]; then echo "   ✅ F2 链接命令计数=8（判据真的在跑，不是空集）"; ok=$((ok+1))
    else echo "   ❌ F2 链接命令计数应为 8，实得 $n_links"; bad=$((bad+1)); fi

    if [ "$n_exempt_var" = 1 ]; then echo "   ✅ F3 变量豁免计数=1（豁免不静默）"; ok=$((ok+1))
    else echo "   ❌ F3 变量豁免计数应为 1，实得 $n_exempt_var"; bad=$((bad+1)); fi

    if [ "$n_skip_self" = 0 ]; then echo "   ✅ F4 判据自身不在 fixture 内 ⇒ 不误伤（自身跳过由真仓扫描覆盖）"; ok=$((ok+1))
    else echo "   ❌ F4 判据自身跳过计数应为 0，实得 $n_skip_self"; bad=$((bad+1)); fi

    # N1 负控：判据自伤（合规正则恒真）⇒ 真实仓库扫描的违例必须归零（判据确有牙）
    local inj="$T/inj.sh"
    cp "$0" "$inj"
    sed -i "s|^COMPLIANT_RX=.*|COMPLIANT_RX='.'|" "$inj"
    if grep -q "^COMPLIANT_RX='\.'" "$inj"; then
        got="$(bash "$inj" "$T/proj" 2>&1 | grep -cE '^   ❌ [a-z]' || true)"
        if [ "$got" = 0 ]; then echo "   ✅ N1 负控：合规正则恒真 ⇒ 违例归零（判据确有牙）"; ok=$((ok+1))
        else echo "   ❌ N1 负控：自伤后仍报 $got 条违例"; bad=$((bad+1)); fi
    else
        echo "   ❌ N1 负控：注入失败（锚点 COMPLIANT_RX 找不到）"; bad=$((bad+1))
    fi

    rm -rf "$T"
    echo "── 小计：通过 $ok · 失败 $bad"
    [ "$bad" = 0 ] || return 1
    return 0
}

for a in "$@"; do
    case "$a" in
        --self-test) SELF_TEST=1 ;;
        --list)      LIST_ONLY=1 ;;
        -h|--help)   sed -n '2,32p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *)           ROOT="${a%/}" ;;
    esac
done

if [ "$SELF_TEST" = 1 ]; then self_test; exit $?; fi
if [ "$LIST_ONLY" = 1 ]; then
    printf '扫描面：%s\n' "${SCAN_GLOBS[@]}"
    printf '变量豁免：%s\n' "${EXEMPT_VARS[@]}"
    printf '行级豁免：%s\n' "${EXEMPT_LINES[@]}"
    exit 0
fi

run_scan
check_exempt_stale
report
