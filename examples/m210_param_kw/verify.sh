#!/usr/bin/env bash
# ============================================================
# M210 门（第 89 轮）：**保留字作「形参名」漏拦截**（缺陷 283）
# ------------------------------------------------------------
# 主题：`expect_name(what)` 是**形参名 / 成员名**两条路径共用的取名字入口，
#   它有一条「放行整张 `is_name_kind` 列表」的后门 ⇒ 两边都很宽松。实际上两面的
#   合理宽松度**本就不同**：
#     · **成员名面必须宽松** —— `ch.send(x)` / `ch.recv()` / `s.type` 的成员名就是关键字
#       （chan 的内建方法名与关键字同名：`cases_bc/bc11`、`bc12`、`registry/workerpool`）；
#     · **形参面应收紧** —— `def f(x, fn)` 此前静默编译通过，而 `var fn` / `for fn in`
#       早已 `E2001` ⇒「**同一名字在不同位置行为不同**」（与 M199 的 [S10] 同族）；
#       更糟的是 `fn` 一被**使用**就报「期望 '('」，错误信息指不到**声明处**的真因。
#
#   修法（`selfhost/parser.px`）：`expect_name(what, allow_kw)` —— 形参面传 `false`，
#   成员名面传 `true`；`self` 例外（方法接收者形参就叫 `self`，见 `capability.px`/`bc8.px`）。
#
# 层的设计（每层都能独立判红）：
#   ① 静态判据：签名有 `allow_kw` 形参、**形参面传 false**、**成员名面传 true**（×3），
#      且**反向**断言「参数名传 true / 成员名传 false」**不存在**（防判据被悄悄改回）
#   ② 拒绝侧（三轨 × 3 探针）：`def f(fn)` / `def f(x, fn)` / `def f(x, y, fn)`
#      ⇒ rc≠0 + `E2001` + 词条**三轨逐字相同**
#   ③ 放行侧（三轨 × 5 探针）：普通形参名 / `extern def …(type: int, …)`（软名字 `type`）/
#      匿名函数 `fn(x): …` / 成员名 `s.type` / 方法接收者 `self`
#   ④ **真实文件不被误伤**（这是本改动的最大风险面）：`selfhost/capability.px`（`def area(self)`）、
#      `registry/workerpool/0.1.0/workerpool.px`（成员名 `send`/`recv`）、`selfhost/cases_bc/bc8.px`
#      ⇒ 三轨全部 rc=0
#   ⑤ **全仓普查**：`selfhost/ stdlib/ tools/ registry/` 全部 `.px`（≈264 件）
#      ⇒ 报「期望参数名/成员名 + 保留字」的 **HIT = 0**（无副作用的最强判据）
#   ⑥ 负控 3 道（`--neg-skip` 可跳；每道**各自独立**判红，且跑前 `restore_all; snapshot` 各自干净起点）：
#      A 退回「共用放行路径」（`if is_name_kind(pk()):`）⇒ ② 不再报错 ⇒ 必红
#      B 形参面改传 `true` ⇒ ② 必红
#      C 去掉 `self` 例外 ⇒ ④ 的 `capability.px` 编译失败 ⇒ 必红
#   ⑦ 源逐字节还原断言
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="${M210_W:-$(mktemp -d /tmp/m210_gate.XXXXXX)}"
mkdir -p "$W"
SNAP="$W/snap"; SRC="selfhost/parser.px"

snapshot() { mkdir -p "$SNAP/selfhost"; cp "$SRC" "$SNAP/$SRC"; }
restore_all() { [ -f "$SNAP/$SRC" ] && cp "$SNAP/$SRC" "$SRC"; }
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PXC="$ROOT/bootstrap/pxc"
PXCVM="$ROOT/bootstrap/pxc_vm"
PXI="$ROOT/bootstrap/pxi"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS + 1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL + 1)); }

# 跑一个探针，回显 rc（stderr 落日志）
run_track() {
    local t="$1" f="$2" rc=0
    local base; base="$(basename "$f" .px)"
    local log="$W/log_${base}_${t}.txt"
    case "$t" in
        C)  "$PXC"   build "$f" >/dev/null 2>"$log" || rc=$? ;;
        VM) "$PXCVM" build "$f" >/dev/null 2>"$log" || rc=$? ;;
        I)  "$PXI"   run   "$f" >/dev/null 2>"$log" || rc=$? ;;
    esac
    echo "$rc"
}

echo "══ M210 门：保留字形参名漏拦截（缺陷 283）══"

# ---------- ① 静态判据 ----------
echo "── [1] 静态判据（源码在位 + 反向断言）"
if grep -q 'def expect_name(what, allow_kw):' "$SRC"; then ok "[1] expect_name 已带 allow_kw 形参"; else bad "[1] expect_name 签名未收紧"; fi
n_p_false=$(grep -c 'expect_name("参数名", false)' "$SRC")
n_m_true=$(grep -c 'expect_name("成员名", true)' "$SRC")
[ "$n_p_false" = "1" ] && ok "[1] 形参面 1 处传 false" || bad "[1] 形参面 false 调用点数=$n_p_false（应为 1）"
[ "$n_m_true" = "3" ] && ok "[1] 成员名面 3 处传 true" || bad "[1] 成员名面 true 调用点数=$n_m_true（应为 3）"
if grep -q 'expect_name("参数名", true)' "$SRC" || grep -q 'expect_name("成员名", false)' "$SRC"; then
    bad "[1] 反向断言：出现了「参数名→true」或「成员名→false」"
else
    ok "[1] 反向断言通过（两面未串用）"
fi
if grep -q 'allow_kw or pk() == "self"' "$SRC"; then ok "[1] self 例外在位"; else bad "[1] 缺少 self 例外"; fi

# ---------- ② 拒绝侧：保留字作形参名 ----------
echo "── [2] 拒绝侧（三轨 × 3 探针：必须 rc≠0 + E2001 + 同词条）"
mkbad() {
    local n="$1" params="$2"
    cat > "$W/b_$n.px" <<EOF
def f($params):
    return 0
def main():
    print(f(1, 2, 3))
    return 0
EOF
}
mkbad p1 "fn"
mkbad p2 "x, fn"
mkbad p3 "x, y, fn"
EXPECT_MSG='期望参数名，实际得到 fn'
for n in p1 p2 p3; do
    msgs=""
    rcs=""
    for t in C VM I; do
        rc=$(run_track "$t" "$W/b_$n.px")
        rcs="$rcs$rc,"
        m=$(grep -o "E2001: $EXPECT_MSG" "$W/log_b_${n}_${t}.txt" | head -1)
        msgs="$msgs|$m"
    done
    [ "$rcs" = "1,1,1," ] && ok "[2] $n 三轨 rc 全非零（$rcs）" || bad "[2] $n 三轨 rc=$rcs（应 1,1,1,）"
    cnt=$(echo "$msgs" | tr '|' '\n' | grep -c "$EXPECT_MSG")
    [ "$cnt" = "3" ] && ok "[2] $n 三轨词条逐字相同" || bad "[2] $n 词条不一致：$msgs"
done

# ---------- ③ 放行侧 ----------
echo "── [3] 放行侧（三轨 × 5 探针：必须 rc=0）"
cat > "$W/g_cb.px" <<'EOF'
def f(cb):
    return cb
def main():
    print(f(7))
    return 0
EOF
cat > "$W/g_soft.px" <<'EOF'
extern def ef(type: int, payload) -> bytes
def main():
    print("soft-name-ok")
    return 0
EOF
cat > "$W/g_anon.px" <<'EOF'
def main():
    var h = fn(x): x + 1
    print(h(41))
    return 0
EOF
cat > "$W/g_self.px" <<'EOF'
struct P:
    v: int
impl P:
    def area(self) -> int:
        return self.v
def main():
    var p = P(3)
    print(p.area())
    return 0
EOF
# 注：**成员名面**的关键字放行（`ch.send` / `ch.recv`）不在本层用合成探针测 ——
#   那是 chan 内建方法名，需要并发（解释轨设计性不支持），故交由 [4] 的**真实文件**覆盖
#   （`registry/workerpool`、`cases_bc/bc11`、`bc12`）。
for n in cb soft anon self; do
    rcs=""
    for t in C VM I; do rc=$(run_track "$t" "$W/g_$n.px"); rcs="$rcs$rc,"; done
    [ "$rcs" = "0,0,0," ] && ok "[3] g_$n 三轨 rc=0" || bad "[3] g_$n 三轨 rc=$rcs（应 0,0,0,）$(head -1 "$W/log_g_${n}_VM.txt")"
done

# ---------- ④ 真实文件不被误伤 ----------
echo "── [4] 真实文件不被误伤（三轨 rc=0）"
for f in selfhost/capability.px registry/workerpool/0.1.0/workerpool.px selfhost/cases_bc/bc8.px; do
    rcs=""
    for t in C VM I; do rc=$(run_track "$t" "$ROOT/$f"); rcs="$rcs$rc,"; done
    [ "$rcs" = "0,0,0," ] && ok "[4] $f 三轨 rc=0" || bad "[4] $f 三轨 rc=$rcs $(head -1 "$W/log_$(basename "$f" .px)_VM.txt")"
done
# [4b] `cases_bc/bc11|bc12` 是**字节码轨专用**用例（含 `spawn`/`chan`）⇒ 解释轨因
#      「interp 不支持通道（Mini 子集排除 · PX-DEF-006）」**设计性**报错，不是本改动的锅。
#      这里的判据因此精确到**「成员名面不得出 E2001」**：
for f in selfhost/cases_bc/bc11.px selfhost/cases_bc/bc12.px; do
    hits=""
    for t in C VM I; do
        run_track "$t" "$ROOT/$f" >/dev/null
        m=$(grep -o 'E2001: 期望成员名[^"]*' "$W/log_$(basename "$f" .px)_${t}.txt" | head -1)
        hits="$hits|$m"
    done
    nhit=$(echo "$hits" | tr '|' '\n' | grep -c 'E2001')
    if [ "$nhit" = "0" ]; then ok "[4b] $f 三轨均无「期望成员名」E2001（成员名面宽松）"; else bad "[4b] $f 命中 $nhit 处：$hits"; fi
done

# ---------- ⑤ 全仓普查 ----------
echo "── [5] 全仓普查（HIT 必须 0）"
census() {   # $1=C 轨编译器路径  → 回显 HIT 清单
    local cc="$1"
    find selfhost stdlib tools registry -name '*.px' -not -path '*/build/*' | sort \
    | xargs -P 4 -I{} bash -c '
        cc="$1"; f="$2"
        err=$("$cc" build "$f" 2>&1 >/dev/null)
        case "$err" in
          *"期望参数名，实际得到"*|*"期望成员名，实际得到"*|*"期望变量名，实际得到"*|*"期望循环变量，实际得到"*)
            echo "HIT $f :: $(echo "$err" | head -1)" ;;
        esac
      ' _ "$cc" {}
}
HITS=$(census "$PXC")
if [ -z "$HITS" ]; then ok "[5] 普查 HIT=0（$(find selfhost stdlib tools registry -name '*.px' -not -path '*/build/*' | wc -l) 件）"; else bad "[5] 普查命中：$(echo "$HITS" | head -3 | tr '\n' ' ')"; fi

# ---------- ⑥ 负控 ----------
if [ "$NEG_SKIP" = "1" ]; then
    echo "── [6] 负控：--neg-skip（跳过）"
else
    echo "── [6] 负控 3 道（各自独立判红）"
    neg_dev() {   # 造一枚「含当前源码」的开发编译器（≈19s）；**强制重建 + 新鲜度断言**
        rm -f /tmp/pxcdev
        ./selfhost/devbuild.sh > "$W/devbuild.log" 2>&1 || return 1
        [ -x /tmp/pxcdev ] || return 1
        # 防「跑的还是旧件」导致负控假绿（实测踩过：带额外参数调用时 pxcdev 不会被重建）
        [ /tmp/pxcdev -nt "$SRC" ] || return 1
        return 0
    }
    neg_A() {   # 退回共用放行路径
        perl -0pi -e 's/if is_name_kind\(pk\(\)\) and \(allow_kw or pk\(\) == "self"\):/if is_name_kind(pk()):/' "$SRC"
        grep -q 'if is_name_kind(pk()):' "$SRC" || return 1
        neg_dev || return 1
        rc=$(/tmp/pxcdev "$W/b_p2.px" 2>&1 >/dev/null; echo $?)
        [ "$rc" != "0" ] && return 1    # 修前会 rc=0；仍非零 ⇒ 负控无效
        return 0                        # rc=0 ⇒ 缺陷复现 ⇒ 判红成立
    }
    neg_B() {   # 形参面改传 true
        sed -i 's/expect_name("参数名", false)/expect_name("参数名", true)/' "$SRC"
        grep -q 'expect_name("参数名", true)' "$SRC" || return 1
        neg_dev || return 1
        rc=$(/tmp/pxcdev "$W/b_p2.px" 2>&1 >/dev/null; echo $?)
        [ "$rc" = "0" ] && return 0 || return 1
    }
    neg_C() {   # 去掉 self 例外
        sed -i 's/ and (allow_kw or pk() == "self")/ and (allow_kw)/' "$SRC"
        grep -q 'and (allow_kw):' "$SRC" || return 1
        neg_dev || return 1
        rc=$(/tmp/pxcdev "$ROOT/selfhost/capability.px" 2>&1 >/dev/null; echo $?)
        [ "$rc" != "0" ] && return 0 || return 1
    }
    for n in A B C; do
        restore_all; snapshot
        if "neg_$n"; then ok "[6] 负控 $n 已判红（符合预期）"; else bad "[6] 负控 $n 未判红（锚点失效或改动无效）"; fi
    done
    restore_all
fi

# ---------- ⑦ 源还原 ----------
echo "── [7] 源逐字节还原"
if cmp -s "$SRC" "$SNAP/$SRC"; then ok "[7] parser.px 逐字节还原"; else bad "[7] parser.px 未还原"; fi
if [ -z "$(git status --porcelain -- "$SRC")" ] || grep -q 'allow_kw' "$SRC"; then ok "[7] 工作区源码形态正常"; else bad "[7] 工作区源码异常"; fi

echo
echo "M210 门结果：PASS=$PASS FAIL=$FAIL"
if [ "$FAIL" = "0" ]; then echo "M210-VERIFY-OK"; exit 0; else echo "M210-VERIFY-FAIL"; exit 1; fi
