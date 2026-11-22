#!/usr/bin/env bash
# ============================================================
# selfhost/check_px_deps.sh —— `px --deps` 的依赖闭包与缓存 key（M436-2 · 缺陷 906）
# ------------------------------------------------------------
# 来历（晨曦 CX 请求 2026-11-23 03:22）：她的 Ma（构建调度）按「脚本所在目录下的
#   **全部** .px」算缓存 key ⇒ 改一个脚本要重编同目录 17 个；一站 200 脚本时
#   改一处 = 200 次重建 + 640MB 产物轮转。她要「与 px run --fast 缓存 key 同一真源」
#   的只读出口，不另写一份 import 解析器。
#
# 判据（每条都能判红）：
#   D0  环境守卫：tools/px · python3 缺失 ⇒ rc=3（不降级）
#   D1  单文件：deps=1 · key 是 16 位十六进制 · rc=0
#   D2  递归闭包：main→lib→sub/deep（3 个）· BFS 顺序第一行是主脚本 · 绝对路径
#   D3  **同一真源**：deps 的 key == `px run --fast` 实际建的缓存目录名
#       （在隔离的 PX_RUNCACHE_DIR 下跑，零污染）—— 这条是「两份实现不漂移」的硬证
#   D4  无关文件不改变 key（晨曦的痛点：同目录加/改**不相干**文件不许重建）
#   D5  依赖内容变更 ⇒ key 必变（反向：改 sub/deep.px 必须重建）
#   D6  --json 形态：字段 file/key/deps 齐全、deps 条数与文本模式一致、路径绝对
#   D7  只读：跑前跑后 .runcache 目录数与仓库 git status 不变（不建缓存、不编译）
#   D8  错误路径：文件不存在 rc=1 · 无参 rc=2
#   N1  负控：把「从 px_dep_closure 取闭包」换成「只有主脚本」⇒ D2/D5 必须判红
#       （证明这两条确实在验闭包，不是在验别的东西）+ 前提自证（补丁真落地）
#   G1  零污染：tools/px sha256 与 git status 行数前后一致
#   G2  收尾：本门不留临时目录/进程
#
# 用法：check_px_deps.sh
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m436_deps.XXXXXX")"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }

[ -x "$PX" ] || { echo "M436-DEPS-ENV-MISSING: $PX"; exit 3; }
command -v python3 >/dev/null 2>&1 || { echo "M436-DEPS-ENV-MISSING: python3"; exit 3; }

FIX="$TMP/fix"
mkdir -p "$FIX/sub"
cat > "$FIX/main.px" <<'PXEOF'
import "./lib.px"

print(str(lib_val()))
PXEOF
cat > "$FIX/lib.px" <<'PXEOF'
import "./sub/deep.px"

def lib_val():
    return deep_val()
PXEOF
cat > "$FIX/sub/deep.px" <<'PXEOF'
def deep_val():
    return 42
PXEOF

export PX_RUNCACHE_DIR="$TMP/runcache"

key_of() { "$PX" --deps "$1" 2>/dev/null | awk '/^key /{print $2}'; }
deps_n() { "$PX" --deps "$1" 2>/dev/null | awk '/^deps /{print $2}'; }

# ── D1 单文件 ─────────────────────────────────────────────
OUT1="$("$PX" --deps "$FIX/sub/deep.px" 2>&1)"; RC1=$?
N1="$(printf '%s\n' "$OUT1" | awk '/^deps /{print $2}')"
K1="$(printf '%s\n' "$OUT1" | awk '/^key /{print $2}')"
if [ "$RC1" = 0 ] && [ "$N1" = 1 ] && printf '%s' "$K1" | grep -Eq '^[0-9a-f]{16}$'; then
    ok "D1 单文件：deps=1 · key=${K1}（16 位十六进制）· rc=0"
else
    bad "D1 单文件异常（rc=$RC1 deps=${N1:-?} key=${K1:-?}）"
fi

# ── D2 递归闭包 ───────────────────────────────────────────
N2="$(deps_n "$FIX/main.px")"
FIRST="$("$PX" --deps "$FIX/main.px" 2>/dev/null | sed -n '4p' | awk '{print $2}')"
HAS_DEEP="$("$PX" --deps "$FIX/main.px" 2>/dev/null | grep -c "sub/deep.px$")"
ALL_ABS="$("$PX" --deps "$FIX/main.px" 2>/dev/null | sed -n '4,$p' | awk '$2 !~ /^\//{c++} END{print c+0}')"
if [ "$N2" = 3 ] && [ "$FIRST" = "$FIX/main.px" ] && [ "$HAS_DEEP" = 1 ] && [ "$ALL_ABS" = 0 ]; then
    ok "D2 递归闭包：deps=3（main→lib→sub/deep）· 首行主脚本 · 全绝对路径"
else
    bad "D2 闭包不符（deps=${N2:-?} 首行=${FIRST:-?} deep命中=$HAS_DEEP 非绝对=$ALL_ABS）"
fi

# ── D3 同一真源（deps key == --fast 缓存目录名）──────────
K3="$(key_of "$FIX/main.px")"
"$PX" run --fast "$FIX/main.px" > "$TMP/fast.out" 2>/dev/null
FAST_OUT="$(cat "$TMP/fast.out")"
if [ -d "$PX_RUNCACHE_DIR/$K3" ] && [ "$FAST_OUT" = 42 ]; then
    ok "D3 同一真源：deps key=${K3} == --fast 缓存目录名（fast 输出 42）"
else
    bad "D3 真源不一致（key=${K3:-?} · 缓存目录 $(ls "$PX_RUNCACHE_DIR" 2>/dev/null | tr '\n' ' ') · fast 输出 ${FAST_OUT:-?}）"
fi

# ── D4 无关文件不改变 key ────────────────────────────────
printf 'zzz = 1\n' > "$FIX/unrelated.px"
K4="$(key_of "$FIX/main.px")"
if [ "$K4" = "$K3" ]; then
    ok "D4 无关文件不影响 key（$K3 不变）—— 同目录加文件不必重建"
else
    bad "D4 无关文件改变了 key（$K3 → ${K4:-?}）"
fi
rm -f "$FIX/unrelated.px"

# ── D5 依赖内容变更 ⇒ key 变 ─────────────────────────────
printf 'def deep_val():\n    return 43\n' > "$FIX/sub/deep.px"
K5="$(key_of "$FIX/main.px")"
if [ -n "$K5" ] && [ "$K5" != "$K3" ]; then
    ok "D5 依赖变更改 key（$K3 → $K5）—— 改了 sub/deep.px 必须重建"
else
    bad "D5 依赖变更后 key 未变（${K5:-?}）⇒ 会跑旧产物"
fi
printf 'def deep_val():\n    return 42\n' > "$FIX/sub/deep.px"

# ── D6 --json 形态 ───────────────────────────────────────
J="$("$PX" --deps --json "$FIX/main.px" 2>/dev/null)"
echo "$J" | python3 -c '
import json, sys
d = json.load(sys.stdin)
assert set(d) == {"file","key","deps"}, d.keys()
assert d["file"].startswith("/"), d["file"]
assert len(d["key"]) == 16
assert len(d["deps"]) == 3, len(d["deps"])
for x in d["deps"]:
    assert set(x) == {"path","sha256_16"}, x
    assert x["path"].startswith("/"), x
    assert len(x["sha256_16"]) == 16, x
' 2>/dev/null && ok "D6 --json：字段齐全 · deps=3 · 路径绝对 · sha16" \
              || bad "D6 --json 形态不符（$J）"

# ── D7 只读 ─────────────────────────────────────────────
CACHE0="$(ls "$PX_RUNCACHE_DIR" 2>/dev/null | wc -l)"
DK1="$(key_of "$FIX/main.px")"
CACHE1="$(ls "$PX_RUNCACHE_DIR" 2>/dev/null | wc -l)"
GIT0="$(cd "$ROOT" && git status --porcelain 2>/dev/null | wc -l)"
DK2="$(key_of "$FIX/main.px")"
GIT1="$(cd "$ROOT" && git status --porcelain 2>/dev/null | wc -l)"
if [ "$CACHE0" = "$CACHE1" ] && [ "$GIT0" = "$GIT1" ] && [ -n "$DK1" ] && [ "$DK1" = "$DK2" ]; then
    ok "D7 只读：不建缓存条目（$CACHE0 → $CACHE1）· 仓库未变（$GIT0 → $GIT1）"
else
    bad "D7 有副作用（缓存 $CACHE0 → $CACHE1 · git $GIT0 → $GIT1）"
fi

# ── D8 错误路径 ─────────────────────────────────────────
"$PX" --deps "$TMP/does_not_exist.px" >/dev/null 2>&1; E1=$?
"$PX" --deps >/dev/null 2>&1; E2=$?
if [ "$E1" = 1 ] && [ "$E2" = 2 ]; then
    ok "D8 错误路径：不存在 rc=1 · 无参 rc=2（不静默成功）"
else
    bad "D8 错误码不符（不存在 rc=$E1 期望 1 · 无参 rc=$E2 期望 2）"
fi

# ── N1 负控：闭包退化成「只有主脚本」────────────────────
PX_BAK="$TMP/px.bak"
SHA0="$(sha256sum "$PX" | cut -d' ' -f1)"
cp -p "$PX" "$PX_BAK"
PATCHED=0
restore_px() { [ "$PATCHED" = 1 ] && cp -p "$PX_BAK" "$PX" && PATCHED=0; }
trap 'restore_px; rm -rf "$TMP"' EXIT

python3 - "$PX" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '    while IFS= read -r d; do [ -n "$d" ] && deps+=("$d"); done < <(px_dep_closure "$abs")\n'
new = '    deps+=("$abs")\n'
n = s.count(old)
if n != 1:
    print("ANCHOR-MISMATCH %d" % n); sys.exit(3)
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PY
if [ $? -ne 0 ]; then
    bad "N1 负控补丁未落地（锚点不唯一）"
else
    PATCHED=1
    # 前提自证必须看**这一处**：别的路径（cmd_run_fast 沙箱复制）也在调 px_dep_closure，
    # 用宽 grep 会假红 —— 判据要贴本次扰动的具体锚点（命中 = 补丁在位 = 前提成立）。
    if grep -q 'deps+=("\$abs")' "$PX"; then
        ok "N1 前提自证：负控已把闭包退化为「只有主脚本」"
        NN="$(deps_n "$FIX/main.px")"
        NK="$(key_of "$FIX/main.px")"
        if [ "$NN" = 1 ]; then
            ok "N1 判红：闭包退化为 1（应 3）⇒ D2 的判据有牙"
        else
            bad "N1 未判红（deps=${NN:-?}，期望 1）"
        fi
    else
        bad "N1 前提不成立（补丁未落地：deps+=(\"\$abs\") 不存在）"
    fi
fi
restore_px

# ── N2 负控 B：run_cache_key 不含依赖内容 ⇒ D5 必红（改依赖 key 不变）──
#   为什么单独做一条：D2 验「闭包列得全」，D5 验「key 含闭包内容」，两者是不同实现
#   （cmd_deps 的展示 vs run_cache_key 的算 key）⇒ 各自需要自己的负控。
python3 - "$PX" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = """        while IFS= read -r _df; do
            [ "$_df" = "$1" ] && continue
            sha256sum "$_df" 2>/dev/null | cut -c1-16
        done < <(px_dep_closure "$1") | LC_ALL=C sort
"""
n = s.count(old)
if n != 1:
    print("ANCHOR-MISMATCH %d" % n); sys.exit(3)
open(p, 'w', encoding='utf-8').write(s.replace(old, ""))
PY
if [ $? -ne 0 ]; then
    bad "N2 负控补丁未落地（锚点不唯一）"
else
    PATCHED=1
    # 前提自证贴本次扰动点：`[ "$_df" = "$1" ] && continue` 只在 run_cache_key 里出现
    # （另一个 `sha256sum "$_df"` 在别的函数里，用宽 grep 会假红）
    if grep -q '\[ "\$_df" = "\$1" \] && continue' "$PX"; then
        bad "N2 前提不成立（补丁后依赖 sha 循环仍在）"
    else
        ok "N2 前提自证：run_cache_key 的依赖内容行已摘除"
        # ⚠️ 比较对象必须是**同一扰动下的两次测量**：patch 后 key 的绝对值和 K3 本就不同
        #   （输入流少了一行），拿 patch 后的 key 去比 K3 会假红 —— 判据要贴契约。
        printf 'def deep_val():\n    return 42\n' > "$FIX/sub/deep.px"
        NKA="$(key_of "$FIX/main.px")"
        printf 'def deep_val():\n    return 99\n' > "$FIX/sub/deep.px"
        NKB="$(key_of "$FIX/main.px")"
        printf 'def deep_val():\n    return 42\n' > "$FIX/sub/deep.px"
        if [ -n "$NKA" ] && [ "$NKA" = "$NKB" ]; then
            ok "N2 判红：key 不含依赖内容时改 deep.px 仍得 $NKA（不变）⇒ D5 的判据有牙"
        else
            bad "N2 未判红（改依赖后 key $NKA → ${NKB:-?}，期望不变）"
        fi
    fi
    restore_px
fi

# ── G1 零污染 ───────────────────────────────────────────
SHA1="$(sha256sum "$PX" | cut -d' ' -f1)"
if [ "$SHA0" = "$SHA1" ]; then
    ok "G1 tools/px 原样回滚（sha ${SHA0:0:12}）"
else
    bad "G1 tools/px 未回滚（sha ${SHA0:0:12} → ${SHA1:0:12}）"
fi

echo
echo "=== m436_px_deps：PASS $PASS / FAIL $FAIL ==="
if [ "$FAIL" = 0 ]; then echo "M436-PX-DEPS-OK"; exit 0; fi
echo "M436-PX-DEPS-FAIL"; exit 1
