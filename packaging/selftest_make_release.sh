#!/usr/bin/env bash
# ============================================================
# packaging/selftest_make_release.sh —— 发布包组装自测（qg-issue 56）
# ------------------------------------------------------------
# 离线自测（不联网、不打 rpm），断言五件事：
#   ① 发布包里**不含** gitignore 的构建产物（tools/build/，回归即红）
#   ② 条目数 == git 跟踪集合（tools/bootstrap/stdlib/runtime − 显式排除 + LICENSE/VERSION/RELEASE.md）
#      ⇒ 本机打包与 CI 打包**内容同构**，不再出现"同 tag 两个 sha256"
#   ③ 符号链接 tools/pxc -> px 在包内仍是链接（不是被解引用的副本）
#   ④ 同一 commit 连打两次 ⇒ sha256 逐字节相同（位级可复现）
#   ⑤ 老 tar（无 --sort=name，如 CentOS 7 的 tar 1.26）下**仍能出包**且**显式告警**
#      —— qg-issue 68：el7 RPM 轨曾因该选项以 exit 64 整条红，且报错被管道吞掉
# 用法: bash packaging/selftest_make_release.sh
# 退出码: 0 = 全部通过；非 0 = 失败用例数
#
# ⚠️ 实现注意（本文件自身的坑，勿"优化"掉）：
#   本脚本 set -o pipefail，而 `tar ... | grep -q X` 中 grep 命中即退出 →
#   tar 收到 SIGPIPE(141) → **整条管道非零**，即使 grep 已命中 ⇒ 判据会**假红**。
#   故此处一律先把 tar 清单存进变量，再用 case/while 在 bash 内判定，不接 grep -q。
# ============================================================
set -uo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

[ -f tools/make_release.sh ] || { echo "❌ 缺 tools/make_release.sh"; exit 1; }
bash -n tools/make_release.sh || { echo "❌ tools/make_release.sh 语法错误"; exit 1; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

# ---- 期望条目数：以 git 索引为准（自适应，不写死数字，防日后目录增减导致假红）----
N_TRACKED="$(git ls-files tools bootstrap stdlib runtime registry | wc -l)"
#   − tools/make_release.sh、tools/install.sh（脚本自身不进包）
#   + LICENSE、VERSION、RELEASE.md（组装期生成）
N_EXPECT=$((N_TRACKED - 2 + 3))

echo "== 自测: 发布包组装（Issue 56）=="
echo "   跟踪件数: $N_TRACKED → 期望包内条目: $N_EXPECT"

# ---- 第一次构建（--no-check：跳过解包冒烟；本自测只验"内容与可复现"）----
mkdir -p "$TMP/a" "$TMP/b"
PKG_A="$TMP/a/pkg.tar.gz"
if ! tools/make_release.sh --no-check -o "$PKG_A" >"$TMP/build1.log" 2>&1; then
    bad "make_release.sh 第一次构建失败"
    sed 's/^/       | /' "$TMP/build1.log"
    echo "== 自测结果: $pass 通过 / $fail 失败 =="; exit "$fail"
fi
[ -f "$PKG_A" ] || { bad "未生成 tarball"; echo "== 自测结果: $pass 通过 / $fail 失败 =="; exit "$fail"; }
ok "make_release.sh 构建成功（$(( $(stat -c %s "$PKG_A") / 1048576 )) MiB）"

# 一次性取两份清单：名字表 + 详细表（含 mode/链接箭头），后续判定全在 bash 内做
LIST="$(tar tzf  "$PKG_A")"
LISTV="$(tar tvzf "$PKG_A")"
TOP="$(printf '%s\n' "$LIST" | head -1 | cut -d/ -f1)"      # puxian-<ver>-<ms>-<sha>
echo "   包内顶层: $TOP"

# ---- ① 不含构建产物 / 杂物 ----
n_build=0; n_junk=0; n_entry=0
while IFS= read -r l; do
    case "$l" in */) continue ;; esac                        # 目录条目不计入
    n_entry=$((n_entry+1))
    case "$l" in *tools/build/*)      n_build=$((n_build+1)) ;; esac
    case "$l" in *.bak*|*__pycache__*) n_junk=$((n_junk+1)) ;; esac
done <<< "$LIST"

if [ "$n_build" = 0 ]; then
    ok "包内无 tools/build/（gitignore 产物未被打进）"
else
    bad "发布包夹带 tools/build/（Issue 56 回归）：$n_build 项"
fi
if [ "$n_junk" = 0 ]; then
    ok "包内无 *.bak / __pycache__ 杂物"
else
    bad "包内含 *.bak / __pycache__ 杂物：$n_junk 项"
fi

# ---- ② 条目数 == git 跟踪集合 ----
if [ "$n_entry" = "$N_EXPECT" ]; then
    ok "条目数 $n_entry == git 跟踪集合 $N_EXPECT（本机与 CI 内容同构）"
else
    bad "条目数 $n_entry != 期望 $N_EXPECT"
    printf '%s\n' "$LIST" | while IFS= read -r l; do case "$l" in */) ;; *) printf '%s\n' "$l";; esac; done | sort > "$TMP/have.txt"
    { git ls-files tools bootstrap stdlib runtime registry \
        | grep -v '^tools/make_release.sh$' | grep -v '^tools/install.sh$'
      printf '%s\n' LICENSE VERSION RELEASE.md; } | sort | sed "s#^#$TOP/#" > "$TMP/want.txt"
    echo "       | 只在包内: $(comm -23 "$TMP/have.txt" "$TMP/want.txt" | head -8 | tr '\n' ' ')"
    echo "       | 只在期望: $(comm -13 "$TMP/have.txt" "$TMP/want.txt" | head -8 | tr '\n' ' ')"
fi

# ---- ③ 符号链接保持为链接（bash case 判定，避开 grep -q + pipefail 假红）----
case "$LISTV" in
    *"/tools/pxc -> px"*) ok "tools/pxc 在包内仍是符号链接 -> px（未被解引用成副本）" ;;
    *) bad "tools/pxc 未保持为符号链接" ;;
esac

# ---- ③b 包内含 registry/（A：registry 纳入发布物；
#          防「白名单改了但目录其实不存在」这类静默）----
case "$LIST" in
    *"/registry/README.md"*) ok "包内含 registry/（A 生效）" ;;
    *) bad "包内不含 registry/README.md ⇒ A 未生效" ;;
esac
# ---- ③c 包内 registry 回退：**解包内端到端**动态判据（A 的必要配套）----
#   在**解包出来的发布物**里跑 —— 这才是用户拿到的形态（在仓库内跑验的是仓库，
#   而仓库里 registry/ 本就在，**验不出「打包时漏了 registry」**）。
#   ① 不设 PX_REGISTRY ⇒ `pxpkg install` 必须成功（回退生效）
#   ② 显式设不存在的 PX_REGISTRY ⇒ 必须失败（证明回退**不覆盖用户意图**；
#      缺此反证，① 可能只是「回退无条件接管」，那是另一种错）
#   ⚠️ 三条实测教训（首版三条判据 **全错、且错得隐蔽**，勿"简化"回去）：
#      · 必须先 `pxpkg init` —— 否则报「未找到 px.toml」；
#        **错误的原因也能让 ① 「失败」** ⇒ 判据读不出真因
#      · 必须用 `install` 而非 `add` —— `add` 只写 px.toml、**根本不碰 registry**
#        ⇒ ① 因别的错假红、② 因别的错**假绿**（反向判据彻底失去意义）
#      · 版本范围取 registry 里**真实存在**的（写 ^1.0.0 而库里只有 0.1.0 ⇒ 必失败）
XDIR="$TMP/x"; mkdir -p "$XDIR"
XROOT=""
if tar xzf "$PKG_A" -C "$XDIR" 2>"$TMP/xerr.log"; then
    XROOT="$(find "$XDIR" -maxdepth 1 -mindepth 1 -type d | head -1)"
fi
XPKG="$XROOT/tools/pxpkg"
if [ -n "$XROOT" ] && [ -f "$XPKG" ] && [ -x "$XROOT/bootstrap/pxi" ]; then
    reg_case() {   # $1=依赖 spec  $2=PX_REGISTRY 取值（空串=不设）  $3=install 日志
        local d="$TMP/regt.$RANDOM"
        mkdir -p "$d"
        ( cd "$d" || exit 9
          env -u PX_REGISTRY bash "$XPKG" init  >/dev/null 2>&1 || exit 8
          env -u PX_REGISTRY bash "$XPKG" add "$1" >/dev/null 2>&1 || exit 7
          if [ -n "$2" ]; then
              PX_REGISTRY="$2" bash "$XPKG" install >"$3" 2>&1
          else
              env -u PX_REGISTRY bash "$XPKG" install >"$3" 2>&1
          fi )
    }
    if reg_case "semver@^0.1.0" "" "$TMP/r1.log"; then
        ok "解包内 · 不设 PX_REGISTRY ⇒ pxpkg install 成功（包内 registry 回退生效）"
    else
        bad "解包内 · 不设 PX_REGISTRY ⇒ pxpkg install 失败（回退未生效）：$(tail -1 "$TMP/r1.log")"
    fi
    if reg_case "uuid@^0.1.0" "/nonexistent-xyz" "$TMP/r2.log"; then
        bad "解包内 · 显式设无效 PX_REGISTRY 却成功 ⇒ 回退**覆盖了**用户意图"
    else
        ok "解包内 · 显式设无效 PX_REGISTRY ⇒ 失败（回退不覆盖用户意图）"
    fi
else
    echo "  ⏭ ③c 跳过（解包内 bootstrap/pxi 或 tools/pxpkg 不可用）"
fi
# ---- ④ 位级可复现：同 commit 再打一次 ----
PKG_B="$TMP/b/pkg.tar.gz"
if ! tools/make_release.sh --no-check -o "$PKG_B" >"$TMP/build2.log" 2>&1; then
    bad "make_release.sh 第二次构建失败"
    sed 's/^/       | /' "$TMP/build2.log"
else
    H_A="$(sha256sum "$PKG_A" | awk '{print $1}')"
    H_B="$(sha256sum "$PKG_B" | awk '{print $1}')"
    if [ "$H_A" = "$H_B" ]; then
        ok "位级可复现：两次打包 sha256 相同（${H_A:0:16}…）"
    else
        bad "两次打包 sha256 不同 ⇒ 不可复现（A=${H_A:0:16}… B=${H_B:0:16}…）"
    fi
fi

# ---- ⑤ 老 tar 兼容（qg-issue 68）----
#   shim 复刻 CentOS 7 自带 GNU tar 1.26：对 --sort=* 报「unrecognized option」并 exit 64
#   （与 glibc argp 的 EX_USAGE 行为一致；2026-09-14 真机复现结论见 ISSUE 68）。
#   断言：① 不硬失败（仍出货）② 有显式告警（不静默降级）。
REAL_TAR="$(command -v tar)"
OLD="$TMP/oldtar"
mkdir -p "$OLD/bin"
cat > "$OLD/bin/tar" <<SHIM
#!/usr/bin/env bash
for a in "\$@"; do
    case "\$a" in
        --sort=*) echo "tar: unrecognized option '\$a'" >&2; exit 64 ;;
    esac
done
exec "$REAL_TAR" "\$@"
SHIM
chmod +x "$OLD/bin/tar"
PKG_C="$TMP/c/pkg.tar.gz"; mkdir -p "$TMP/c"
if PATH="$OLD/bin:$PATH" tools/make_release.sh --no-check -o "$PKG_C" >"$TMP/oldtar.log" 2>&1; then
    ok "老 tar（无 --sort=name，模拟 el7 tar 1.26）下仍能出包"
    case "$(cat "$TMP/oldtar.log")" in
        *"不支持 --sort=name"*) ok "老 tar 降级有显式告警（未静默降级）" ;;
        *) bad "老 tar 降级无告警 ⇒ 静默降级，现场无从判读" ;;
    esac
else
    bad "老 tar 下打包失败（el7 RPM 轨会红）"
    sed 's/^/       | /' "$TMP/oldtar.log"
fi

echo "== 自测结果: $pass 通过 / $fail 失败 =="
exit "$fail"
