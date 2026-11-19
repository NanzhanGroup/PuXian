#!/bin/bash
# ============================================================
# m420_route_intresp —— 路由 handler 返回非 dict（int 状态码/字符串）时不得崩
# ------------------------------------------------------------
# 缺陷 845（M420 实测，2026-11-19）：
#   `examples/m28_route.px` 里 `def health(req, params): return 200` 是**产品允许**的语义
#   （route_normalize 有 PX_INT 分支，见 runtime_route.c:295）。但 px_route_respond /
#   px_route_mw_short_respond 在归一化之后**无条件** `px_dict_get(resp, "headers")`：
#   非 dict 值的 as.obj 是联合体里的垃圾（实测为 0）⇒ px_dict_get 读 o->type ⇒
#   SEGV on 0x0000000000c8。常规静态构建 3/3 必崩（rc=139），与 ASan 无关。
#
# 门结构（判据 + 够牙的负控）：
#   [1] 静态判据：两处 headers 提取必须落在 `type == PX_DICT` 保护内
#   [2] 静态负控：把「复原成无条件调用」的片段喂给同一检查器 ⇒ 必须判红（证明检查器有牙）
#   [3] 动态正控：真构建 + 真跑 3 次 m28_route ⇒ rc==0 且打印「M28 路由 + 中间件全部通过」
#   [4] 前提自证：打印实跑到的请求行（证明真跑到 int 返回的 /health 之后）
# ============================================================
set -uo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
RT=$(cd "$HERE/../.." && pwd)
ROUTE_C="$RT/runtime/runtime_route.c"
SRC="$RT/examples/m28_route.px"
pass=0; fail=0
ok()  { pass=$((pass+1)); echo "PASS $*"; }
bad() { fail=$((fail+1)); echo "FAIL $*"; }

# ---------- 检查器：扫一个文件，两处 headers 提取是否被类型判断保护 ----------
# 用法：check_guard <file>  → 输出 "GUARD pos=<行号>" 列表；未保护则 "UNGUARDED pos=<行号>"
check_guard() {
  awk '
    /px_dict_get\((resp|r),[[:space:]]*"headers"\)/ {
      guarded = 0
      for (i = NR - 4; i < NR; i++) if (i > 0 && buf[i % 8] ~ /type[[:space:]]*==[[:space:]]*PX_DICT/) guarded = 1
      if (guarded) printf "GUARD pos=%d\n", NR
      else         printf "UNGUARDED pos=%d\n", NR
    }
    { buf[NR % 8] = $0 }
  ' "$1"
}

# [1] 真源码静态判据
out=$(check_guard "$ROUTE_C")
n_guard=$(printf '%s\n' "$out" | grep -c '^GUARD' || true)
n_bad=$(printf '%s\n' "$out" | grep -c '^UNGUARDED' || true)
echo "  [1] 真源码：guard=$n_guard unguarded=$n_bad"
printf '%s\n' "$out" | sed 's/^/      /'
if [ "$n_guard" -ge 2 ] && [ "$n_bad" -eq 0 ]; then
  ok "[1] 两处 headers 提取均在 PX_DICT 保护内"
else
  bad "[1] 存在未保护的 px_dict_get(<resp>,\"headers\")（缺陷 845 回归）"
fi

# [2] 静态负控：复原成无条件调用的片段 ⇒ 检查器必须判红
tmpd=$(mktemp -d /tmp/m420-route-neg.XXXXXX) || { echo "M420-ROUTE-ENV rc=3"; exit 3; }
cat > "$tmpd/neg.c" <<'NEG'
void px_route_respond_bad(PxHttpOut* out, LXValue req, LXValue resp) {
    RouteResp rr;
    route_normalize(resp, &rr);
    {
        LXValue rh = px_dict_get(resp, "headers");
        if (rh.type == PX_DICT) (void)rh;
    }
}
NEG
negout=$(check_guard "$tmpd/neg.c")
neg_bad=$(printf '%s\n' "$negout" | grep -c '^UNGUARDED' || true)
echo "  [2] 负控（复原片段）：unguarded=$neg_bad"
if [ "$neg_bad" -ge 1 ]; then
  ok "[2] 负控红：复原成无条件调用时检查器判红（前提自证：检查器真在看类型判断）"
else
  bad "[2] 负控不红 —— 检查器没有牙，[1] 的绿不可信"
fi
rm -rf "$tmpd"

# [3] 动态正控：真构建 + 真跑 3 次
if [ ! -f "$SRC" ]; then
  echo "M420-ROUTE-ENV rc=3（缺 $SRC）"; exit 3
fi
build_log=/tmp/m420-route-build.$$.log
if ! "$RT/tools/px" build --no-quic "$SRC" > "$build_log" 2>&1; then
  bad "[3] 构建失败（见 $build_log）"; tail -5 "$build_log"
  echo "M420-ROUTE rc=3 pass=$pass fail=$fail"; exit 3
fi
BIN="$RT/examples/build/m28_route"
[ -x "$BIN" ] || { echo "M420-ROUTE-ENV rc=3（缺产物 $BIN）"; exit 3; }

runs_bad=0
last_log=""
for i in 1 2 3; do
  lg=/tmp/m420-route-run$i.$$.log
  timeout 60 "$BIN" > "$lg" 2>&1
  rc=$?
  if [ "$rc" -ne 0 ]; then
    runs_bad=$((runs_bad+1)); echo "  [3] run$i rc=$rc"
  fi
  last_log="$lg"
done
if [ "$runs_bad" -eq 0 ]; then
  ok "[3] 3/3 次运行 rc=0"
else
  bad "[3] $runs_bad/3 次非零退出（rc=139 = SEGV，缺陷 845 形态）"
fi

# [4] 前提自证：必须真的跑到 int 返回值路由（/health -> 200）之后
if grep -q 'M28 路由 + 中间件全部通过' "$last_log" 2>/dev/null; then
  ok "[4] 前提自证：跑到尾部（含 /health int 返回 200、401 中间件短路、404）"
  grep -E '\[route\]' "$last_log" | sed 's/^/      /' | head -12
else
  bad "[4] 未跑到尾部（说明崩在途中）"
  tail -5 "$last_log" 2>/dev/null | sed 's/^/      /'
fi

echo "M420-ROUTE-INT-RESP pass=$pass fail=$fail"
[ "$fail" -eq 0 ] && exit 0
exit 1
