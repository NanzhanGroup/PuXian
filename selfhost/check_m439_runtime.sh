#!/usr/bin/env bash
. "$(dirname "$0")/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
# ============================================================
# M439 门 · 运行时两条缺陷 919 / 920（晨曦上报）
# ------------------------------------------------------------
# 919：sqlite_exec 两参分支成功后硬 `return px_int(0)`，而文件头注释写「返回受影响行数」
#   ⇒ 用返回值判 DML 成败的代码恒判失败（Ma v0.20.14 把刚 rename 的 .body 删掉；
#     ma-cache 失效 22 天 / ~1.57 万请求白做）。方案 A（零破坏）：
#     ① 注释改成实话「成功恒 0 / 出错 null / 判失败只能判 null」；② 新增 sqlite_changes(db)。
#   **不改返回值** —— 存量 `if sqlite_exec(...) == 0` 判失败的代码不能变语义。
# 920：json_parse 的 \uXXXX 不合并 UTF-16 代理对 ⇒ "\ud83d\ude80" 落盘 ED A0 BD ED BA 80
#   （CESU-8，非法 UTF-8，静默），与 Go 的 F0 9F 9A 80 不一致；影响所有 px 程序。
#   修：高代理 + 紧跟低代理合并成 4 字节；孤立代理 → U+FFFD。
#
# 判据（6 组，逐组可单独变红）：
#   [1] 静态形状（注释口径 / 两参分支不取 changes / 新内置已注册 / 合并公式在位 / 4 字节分支在位）
#   [2] 解释轨输出与 m439_runtime.expected 逐字节一致
#   [3] VM 轨 同上
#   [4] C 轨 同上
#   [5] 负控（判据有牙）：把期望副本改成 ① 920 旧 CESU-8 值 ② miss_chg=2 ③ ins_ret=1
#       ⇒ 判据必须报不一致；且每次都先自证「扰动确实发生」
#   [6] 前提自证：内置名册含 sqlite_changes（gen_builtin_list --check 绿），
#       否则「编译器不认识新内置」会让轨道静默跑旧名册
# 用法：bash selfhost/check_m439_runtime.sh
# 退出码：0 = 绿 · 1 = 红 · 2 = 环境不成立
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/.."
ROOT=$(pwd)
PXI=./bootstrap/pxi
PX=./tools/px
CASE=selfhost/probes/m439_runtime.px
EXPECT=selfhost/probes/m439_runtime.expected
export LC_ALL=C LANG=C
for f in "$PXI" "$PX" "$CASE" "$EXPECT"; do
  [ -e "$f" ] || { echo "❌ ENV rc=2 缺 $f"; exit 2; }
done
command -v cc >/dev/null 2>&1 || { echo "❌ ENV rc=2 缺 cc"; exit 2; }
W=$(mktemp -d /tmp/m439r.XXXXXX) || { echo "❌ ENV rc=2 mktemp 失败"; exit 2; }
trap 'rm -rf "$W"' EXIT
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M439 门 · 缺陷 919/920（运行时） =="

# ---------------- [1] 静态形状 ----------------
echo "[1/6] 静态形状"
sq=$(cat runtime/runtime_sqlite.c)
two_arg=$(awk '/if \(nargs == 2\) \{/,/^    \}/' runtime/runtime_sqlite.c)
# 判据读代码不读注释：本分支的注释里**故意**写着「不要改成 sqlite3_changes(db)」，
#   那是警告文案，不是代码 —— 先剥 `//` 行注释再判（否则判据自己造的假红）。
two_code=$(sed 's://.*::' <<< "$two_arg")
chg_fn=$(awk '/^LXValue bi_sqlite_changes\(/,/^}/' runtime/runtime_sqlite.c)
grep -q '成功恒返回 0' <<< "$sq" \
  && ok "文件头注释写明「成功恒返回 0」（不再是「返回受影响行数」）" \
  || bad "文件头注释未写明「成功恒返回 0」"
grep -q '判失败只能判' <<< "$sq" \
  && ok "文件头注释写明「判失败只判 null」" \
  || bad "文件头注释缺「判失败只判 null」"
grep -q 'return px_int(0);' <<< "$two_arg" \
  && ok "两参分支仍返回 0（历史契约未改）" \
  || bad "两参分支未返回 px_int(0)"
if grep -q 'sqlite3_changes' <<< "$two_code"; then
  bad "两参分支出现 sqlite3_changes —— 会破坏存量 ==0 判失败语义（缺陷 919 复发）"
else
  ok "两参分支不取 sqlite3_changes（不破坏存量语义）"
fi
grep -q 'sqlite3_changes(db)' <<< "$chg_fn" \
  && ok "sqlite_changes 实现体取 sqlite3_changes(db)" \
  || bad "sqlite_changes 未见 sqlite3_changes(db)（硬 0 假修）"
grep -q 'px_set_global("sqlite_changes"' runtime/runtime.c \
  && ok "runtime.c 已 px_set_global 注册 sqlite_changes" \
  || bad "runtime.c 未注册 sqlite_changes（全局）"
grep -q 'px_ffi_register("sqlite_changes"' runtime/runtime.c \
  && ok "runtime.c 已 px_ffi_register 注册 sqlite_changes（FFI 表）" \
  || bad "runtime.c 未 px_ffi_register sqlite_changes"
grep -q 'bi_sqlite_changes' runtime/runtime.h \
  && ok "runtime.h 声明 bi_sqlite_changes" \
  || bad "runtime.h 缺 bi_sqlite_changes 声明"
grep -q '0x10000u + ((code - 0xD800u) << 10)' runtime/runtime.c \
  && ok "920 合并公式在位（cp = 0x10000 + (hi-D800)*400 + (lo-DC00)）" \
  || bad "920 合并公式缺失（代理对未合并）"
grep -q '0xF0 | (code >> 18)' runtime/runtime.c \
  && ok "920 四字节 UTF-8 分支在位" \
  || bad "缺四字节 UTF-8 分支（> U+FFFF 写不出去）"
grep -q '0xFFFDu' runtime/runtime.c \
  && ok "920 孤立代理回落 U+FFFD" \
  || bad "缺 U+FFFD 回落（孤立代理会产出 CESU-8）"

# ---------------- [6] 名册（先跑，因为三轨要用） ----------------
echo "[6/6] 内置名册派生无漂移"
if bash tools/gen_builtin_list.sh --check >/dev/null 2>&1; then
  ok "tools/gen_builtin_list.sh --check 绿（名册 = runtime 注册表）"
else
  bad "名册漂移（跑 bash tools/gen_builtin_list.sh）"
fi
grep -q 'sqlite_changes' tools/lint_core.px \
  && ok "lint_core.px 名册含 sqlite_changes" \
  || bad "名册缺 sqlite_changes"

# ---------------- [2] 解释轨 ----------------
echo "[2/6] 解释轨（bootstrap/pxi）"
timeout 120 "$PXI" "$CASE" > "$W/interp.out" 2> "$W/interp.err"
rc=$?
[ "$rc" = 0 ] && ok "解释轨 rc=0" || { bad "解释轨 rc=$rc"; tail -3 "$W/interp.err" | sed 's/^/     /'; }
if cmp -s "$W/interp.out" "$EXPECT"; then
  ok "解释轨输出与 expected 逐字节一致"
else
  bad "解释轨输出与 expected 不一致"; diff "$EXPECT" "$W/interp.out" | head -8 | sed 's/^/     /'
fi

# ---------------- [3] VM 轨 ----------------
echo "[3/6] VM 轨"
cp "$CASE" "$W/p.px"
if timeout 500 "$PX" build "$W/p.px" > "$W/vm.log" 2>&1 && [ -x "$W/build/p" ]; then
  timeout 120 "$W/build/p" > "$W/vm.out" 2>/dev/null
  if cmp -s "$W/vm.out" "$EXPECT"; then
    ok "VM 轨输出与 expected 逐字节一致"
  else
    bad "VM 轨输出与 expected 不一致"; diff "$EXPECT" "$W/vm.out" | head -8 | sed 's/^/     /'
  fi
else
  bad "VM 轨编译失败"; tail -3 "$W/vm.log" | sed 's/^/     /'
fi
rm -rf "$W/build"

# ---------------- [4] C 轨 ----------------
echo "[4/6] C 轨"
if timeout 600 "$PX" build --c "$W/p.px" > "$W/c.log" 2>&1 && [ -x "$W/build/p" ]; then
  timeout 120 "$W/build/p" > "$W/c.out" 2>/dev/null
  if cmp -s "$W/c.out" "$EXPECT"; then
    ok "C 轨输出与 expected 逐字节一致"
  else
    bad "C 轨输出与 expected 不一致"; diff "$EXPECT" "$W/c.out" | head -8 | sed 's/^/     /'
  fi
else
  bad "C 轨编译失败"; tail -3 "$W/c.log" | sed 's/^/     /'
fi

# ---------------- [5] 负控 ----------------
# 判据本身必须有牙：把「期望」改成未修 / 假修时的值，同一比较函数必须报不一致。
echo "[5/6] 负控（判据有牙）"
neg() { # $1 标签 · $2 sed 表达式 · 余下为「扰动前该字符串应出现次数」
  local label="$1" expr="$2"; shift 2
  cp "$EXPECT" "$W/neg.exp"
  sed -i "$expr" "$W/neg.exp"
  if cmp -s "$EXPECT" "$W/neg.exp"; then
    bad "$label · 前提不成立（扰动没发生，负控是空转）"
    return
  fi
  ok "$label · 前提自证：期望副本已改变"
  if cmp -s "$W/interp.out" "$W/neg.exp"; then
    bad "$label · 判据没识破（改成错误期望仍判绿 ⇒ 判据无牙）"
  else
    ok "$label · 判据识破（错误期望 ⇒ 不一致）"
  fi
}
neg "920 旧值（CESU-8 ED A0 BD…）" 's/^920_pair     f09f9a80$/920_pair     eda0bdedba80/'
neg "919 miss_chg 假修（0→2）"    's/^919_miss_chg 0$/919_miss_chg 2/'
neg "919 两参契约（0→1）"         's/^919_ins_ret  0$/919_ins_ret  1/'

echo "──────────────────────────────"
if [ "$fail" -eq 0 ]; then
  echo "M439-RUNTIME-OK  pass=$pass fail=0"
  exit 0
fi
echo "M439-RUNTIME-FAIL  pass=$pass fail=$fail"
exit 1
