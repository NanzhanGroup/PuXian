#!/usr/bin/env bash
# ============================================================
# check_no_secrets.sh —— 仓库**明文凭据**守卫（M282 建立 · 缺陷 488 的兄弟面）
# ------------------------------------------------------------
# 为什么有这道门（缺陷 488 的教训）：
#   `import_registry_px.sh` 曾把 `git config remote.origin.url` **原样**写进
#   `registry/THIRD_PARTY.md` —— 带 PAT 的 clone URL（`https://x-access-token:<pat>@…`）
#   会把**密钥永久写进公开仓库**。沙箱实测：第 4 行出现 `github_pat_…`。
#   488 修的是**源头**；本门守的是**全仓面**。
#
# 两类判据（**刻意收紧**，宁可漏古怪形状也不把正常代码判红）：
#   ① **token 形状**（逐行）：github_pat_ / ghp_ / gho_ / ghu_ / ghs_ / ghr_ / glpat- / AKIA /
#      `x-access-token:<非变量>@`
#   ② **私钥实体**（逐文件）：PEM 头 + **紧跟 ≥100 字符的 base64 体**（只看头会误报：
#      本项目多处把 PEM 头当**字符串字面量**用，实测 3 处假阳）。
#   白名单只允许「**变量引用**」形态（`${TOKEN}` / `${{ secrets.X }}`）—— 扫描前**规范化掉**。
#   私钥夹具走 `selfhost/no_secrets_allow.tsv`（**带理由 + 过期判据**：表里的路径若不再命中 ⇒ 判红）。
#   ⚠️ **token 命中刻意不设允许表**：允许表按**文件**豁免，某文件一旦上表，日后**真的**往它里面
#      写凭据也会被静默放过。⇒ 夹具一律**运行时拼装**（源码里不出现连续凭据形状），判据保持绝对。
#
# ⚠️⚠️ **扫描面 = 已跟踪 ∪ 未跟踪未忽略**（M282 事故后改，别再退回只扫 `git ls-files`）：
#   原实现只扫**已跟踪**文件 ⇒ 新写、还没 `git add` 的文件**不在扫描面里** ⇒
#   门**提交前绿、提交后红**（实测：19:38/19:44 绿 · 21:22 红；6 处命中全是本次新增的门
#   自己的 `verify.sh` 与守卫自己的夹具）⇒「干跑通过 ≠ 提交后通过」（与缺陷 487 同形）。
#   而守卫的职责恰是**提交前**拦住 ⇒ 两种世界必须同一结论。自证 ⑨/⑩ 守这条。
#   （`NO_SECRETS_TRACKED_ONLY=1` = **对照档**，只扫已跟踪 ⇒ 复现修前行为，仅供负控/自证用。）
#
# 用法：selfhost/check_no_secrets.sh [--root DIR] [--self-test] [-v]
# 退出码：0 干净 · 1 有明文凭据 · 2 用法错 · 3 判据自身失效（自证失败）
# ============================================================
set -u
ROOT=""; SELFTEST=0; VERBOSE=0
while [ $# -gt 0 ]; do
    case "$1" in
        --root) ROOT="$2"; shift 2 ;;
        --self-test) SELFTEST=1; shift ;;
        -v) VERBOSE=1; shift ;;
        -h|--help) sed -n '2,22p' "$0"; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done
SELF_DIR="$(cd "$(dirname "$(readlink -f "$0" 2>/dev/null || echo "$0")")" && pwd)"
[ -n "$ROOT" ] || ROOT="$(cd "$SELF_DIR/.." && pwd)"

PATTERNS='github_pat_[A-Za-z0-9_]{20,}|ghp_[A-Za-z0-9]{30,}|gho_[A-Za-z0-9]{30,}|ghu_[A-Za-z0-9]{30,}|ghs_[A-Za-z0-9]{30,}|ghr_[A-Za-z0-9]{30,}|glpat-[A-Za-z0-9_-]{15,}|AKIA[0-9A-Z]{16}|x-access-token:[^$<][^@]{8,}@'
# ⚠️ 体是**每 64 字符换行**的 ⇒ 不能只要求「紧跟那一行 ≥100」（实测漏掉真夹具）。
#   第一次改「头 + 50 字节任意内容 + 尾」⇒ 仍误报：`keygen_test.px` 里**头与尾是两个不相邻的
#   字符串字面量**（中间隔着别的代码 120 字符）也被算成实体（实测真仓 4 → 2）。
#   定稿判据 = 「头 + 换行 + **≥200 字符的纯 base64/换行段**」：
#     · 真夹具（每行 64 字符 body）命中；· 只有头/尾字面量、中间是代码或 `abc` ⇒ **不命中**。
KEYRX='-----BEGIN [A-Z ]*PRIVATE KEY-----\n[A-Za-z0-9+/=\n]{200,}'
ALLOW="${NO_SECRETS_ALLOW:-$SELF_DIR/no_secrets_allow.tsv}"
# 对照档（复现 M282 修前的「只扫已跟踪」行为）—— **仅供自证/负控**，生产与 CI 不得设置。
TRACKED_ONLY="${NO_SECRETS_TRACKED_ONLY:-0}"
if [ "$TRACKED_ONLY" = 1 ]; then
    echo "⚠️ 对照档 NO_SECRETS_TRACKED_ONLY=1：**只扫已跟踪文件** —— 未 git add 的新文件不会被扫（= M282 修前行为，生产/CI 不得使用）" >&2
fi

normalize() {  # 抹掉变量引用形态 ⇒ 正当用法不假红
    sed -E -e 's/\$\{\{[^}]*\}\}/<VAR>/g' -e 's/\$\{[A-Za-z_][A-Za-z0-9_]*\}/<VAR>/g' -e 's/\$[A-Za-z_][A-Za-z0-9_]*/<VAR>/g'
}
filelist() {   # $1=dir → 受管文件清单（**显式 if/else**：`a&&b||c&&d` 在 bash 里是 ((a&&b)||c)&&d，
    local d="$1"  # 那个写法会让两个来源都跑 ⇒ 命中数翻倍。M282 自证当场抓到，别改回去。）
    if (cd "$d" && git rev-parse --git-dir >/dev/null 2>&1); then
        # ⚠️⚠️ **必须含未跟踪未忽略文件**（M282 事故后改，别再退回只扫 `git ls-files`）：
        #   只扫已跟踪 ⇒ 新写、还没 `git add` 的文件不在扫描面里 ⇒ 门**提交前绿、提交后红**。
        #   用 `{ a; b; }` 两条独立命令（**不**踩上面那个 `&&/||` 的坑）· `sort -u` 让顺序确定。
        {
            (cd "$d" && git ls-files 2>/dev/null)
            [ "$TRACKED_ONLY" = 1 ] || (cd "$d" && git ls-files --others --exclude-standard 2>/dev/null)
        } | sort -u
    else
        (cd "$d" && find . -type f -not -path './.git/*' 2>/dev/null | sed 's|^\./||')
    fi
}
scan_tokens() {  # $1=dir → 「文件:行:片段」
    local d="$1" f
    filelist "$d" | while IFS= read -r f; do
        [ -n "$f" ] || continue; [ -f "$d/$f" ] || continue
        normalize < "$d/$f" 2>/dev/null | grep -InE "$PATTERNS" 2>/dev/null | while IFS= read -r hit; do
            printf '%s:%s\n' "$f" "${hit:0:150}"
        done
    done
}
scan_keys() {  # $1=dir → 命中私钥实体的**文件路径**
    local d="$1" f
    filelist "$d" | while IFS= read -r f; do
        [ -n "$f" ] || continue; [ -f "$d/$f" ] || continue
        if grep -Pzoq -- "$KEYRX" "$d/$f" 2>/dev/null; then printf '%s\n' "$f"; fi
    done
}
allow_hit() {  # $1=路径 → 在允许表里则打印理由
    [ -f "$ALLOW" ] || return 1
    awk -F'\t' -v p="$1" '$1==p && $0 !~ /^#/ {print $2; found=1} END{exit !found}' "$ALLOW"
}

if [ "$SELFTEST" = 1 ]; then
    W="$(mktemp -d /tmp/pxnosec.XXXXXX)"; trap 'rm -rf "$W"' EXIT
    ( cd "$W" && git init -q . )
    P=0; F=0
    ck() { if [ "$2" = "$3" ]; then P=$((P+1)); echo "  ✓ $1"; else F=$((F+1)); echo "  ✗ $1（期望 $3，实得 $2）"; fi; }
    emit() { ( cd "$W" && git add -A >/dev/null 2>&1 ); }

    # ⚠️ 假凭据一律**运行时拼装**：源码里不出现连续凭据形状（否则本仓自己的 [5b] 会判红）。
    #    这不是洁癖 —— 编辑器高亮 / GitHub secret scanning / 旁人 copy 都会把真形状当真凭据。
    printf 'url=https://x-access-token:%s@github.com/a/b.git\n' \
        "github_pat_11ABCDEFG0""abcdefghijklmnopqrstuv" > "$W/bad1.txt"; emit
    ck "① 明文 github_pat_ 命中" "$(scan_tokens "$W" | grep -c bad1.txt)" 1
    rm -f "$W/bad1.txt"; printf 'url=https://x-access-token:${TOKEN}@github.com/${GITHUB_REPOSITORY}.git\n' > "$W/ok1.txt"; emit
    ck "② \${TOKEN} 不误报" "$(scan_tokens "$W" | grep -c ok1.txt)" 0
    rm -f "$W/bad1.txt"; printf 'ghp_%s\n' "ABCDEFGHIJKLMNOPQRSTUVWXYZ""0123456789" > "$W/bad2.txt"; emit
    ck "③ ghp_ 命中" "$(scan_tokens "$W" | grep -c bad2.txt)" 1
    rm -f "$W/bad2.txt"; printf 'assert(starts_with(k, "-----BEGIN PRIVATE KEY-----"))\n' > "$W/lit.px"; emit
    ck "④ PEM 头**字面量**不算私钥实体" "$(scan_keys "$W" | grep -c lit.px)" 0
    rm -f "$W/lit.px"
    { printf -- '-----BEGIN PRIVATE KEY-----\n'; head -c 400 /dev/urandom | base64 | tr -d '\n'; printf '\n-----END PRIVATE KEY-----\n'; } > "$W/real.pem"; emit
    ck "⑤ PEM 头+base64 体 ⇒ 命中" "$(scan_keys "$W" | grep -c real.pem)" 1
    printf '%s\t%s\n' "real.pem" "自证夹具（非真凭据）" > "$W/allow.tsv"
    ALLOW="$W/allow.tsv"
    ck "⑥ 允许表生效（有理由的夹具不再报）" "$(scan_keys "$W" | while IFS= read -r p; do allow_hit "$p" >/dev/null 2>&1 || echo "$p"; done | grep -c real.pem)" 0
    ALLOW="/nonexistent.tsv"
    ck "⑥b 表不存在 ⇒ 仍报（不漏）" "$(scan_keys "$W" | while IFS= read -r p; do allow_hit "$p" >/dev/null 2>&1 || echo "$p"; done | grep -c real.pem)" 1
    ALLOW="$SELF_DIR/no_secrets_allow.tsv"
    SAVE="$PATTERNS"; PATTERNS='$^'
    ck "⑦ 判据自伤（模式清空后不报）" "$(scan_tokens "$W" | grep -c real.pem)" 0
    PATTERNS="$SAVE"
    ck "⑧ 还原后 token 面仍有效" "$(printf '%s\n' "github_pat_11ABCDEFG0""abcdefghijklmnopqrstuv" > "$W/bad4.txt"; emit; scan_tokens "$W" | grep -c bad4.txt)" 1
    # ⑨/⑩ **扫描面**判据（M282 事故的立论）：未跟踪文件必须被扫到 ——
    #   否则「提交前绿、提交后红」（真实事故：新门自己的文件当时还没 git add）。
    rm -f "$W/bad3.txt"
    printf 'url=https://x-access-token:%s@github.com/a/b.git\n' \
        "github_pat_11ABCDEFG0""abcdefghijklmnopqrstuv" > "$W/bad3.txt"   # 刻意 **不** emit ⇒ 未跟踪
    ck "⑨ 未跟踪文件也被扫到（提交前 = 提交后）" "$(scan_tokens "$W" | grep -c bad3.txt)" 1
    TRACKED_ONLY=1
    ck "⑩ 对照档（只扫已跟踪）会漏掉它 ⇒ 证明 ⑨ 有牙" "$(scan_tokens "$W" | grep -c bad3.txt)" 0
    TRACKED_ONLY=0
    rm -f "$W/bad3.txt"
    echo "SELFTEST 通过 $P / 失败 $F"
    [ "$F" = 0 ] || exit 3
    exit 0
fi

cd "$ROOT" || { echo "进不去 $ROOT" >&2; exit 2; }
N="$(filelist "$ROOT" | wc -l)"
[ "$N" -ge 200 ] || { echo "✗ 判据自身失效：受管文件只有 $N 个（应 ≥200）" >&2; exit 3; }
TOKS="$(scan_tokens "$ROOT")"; TH="$(printf '%s' "$TOKS" | grep -c . || true)"
KEYS_RAW="$(scan_keys "$ROOT")"
KEYS=""; KH=0
while IFS= read -r p; do
    [ -n "$p" ] || continue
    if allow_hit "$p" >/dev/null 2>&1; then continue; fi
    KEYS="${KEYS}${p}"$'\n'; KH=$((KH+1))
done <<< "$([ -n "$KEYS_RAW" ] && printf '%s\n' "$KEYS_RAW" || true)"
# 允许表过期判据：表里每一条都必须仍被 scan_keys 命中，否则判红（防「表留着当永久免罪符」）
ALLOW_STALE=0
if [ -f "$ALLOW" ]; then
    while IFS=$'\t' read -r ap ar; do
        case "$ap" in ''|'#'*) continue ;; esac
        printf '%s\n' "$KEYS_RAW" | grep -qxF "$ap" || { echo "  ✗ 允许表过期：$ap 已不再命中（理由：$ar）" >&2; ALLOW_STALE=$((ALLOW_STALE+1)); }
    done < "$ALLOW"
fi
if [ "$TH" != 0 ] || [ "$KH" != 0 ] || [ "$ALLOW_STALE" != 0 ]; then
    echo "✗ 明文凭据守卫判红：token 形状 $TH 处 · 私钥实体 $KH 处 · 允许表过期 $ALLOW_STALE 条"
    [ "$TH" != 0 ] && printf '%s\n' "$TOKS" | head -20
    [ "$KH" != 0 ] && printf '%s\n' "$KEYS" | grep -v '^$' | sed 's/^/  私钥实体: /' | head -10
    echo "处置：① 不要提交；② 已提交过 ⇒ **立刻吊销该凭据**；③ 改成环境变量/secret 引用。"
    exit 1
fi
echo "✓ 无明文凭据（受管文件 $N · token 模式 $(printf '%s' "$PATTERNS" | tr '|' '\n' | wc -l) 类 · 私钥实体 0）"
exit 0
