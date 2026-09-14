#!/usr/bin/env bash
# M120 · 字典缺键「严格」口径 + 协程错误不再静默（qg-issue 76）
# 覆盖：E1 协程隔离后退出码 / E2 PX_SPAWN_ISOLATE 逃生舱 / E3 错误码缺 R1007 / E4 非字符串键段错误
# 轨：解释器（px run）· VM 轨（px build 默认）· C 轨（px build --c）
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="$(mktemp -d /tmp/m120_gate.XXXXXX)"
PASS=0; FAIL=0
chk(){ if [ "$2" = "$3" ]; then echo "  ✅ $1"; PASS=$((PASS+1)); else echo "  ❌ $1 — 期望[$3] 实得[$2]"; FAIL=$((FAIL+1)); fi; }
chk_has(){ if grep -q -- "$3" "$2"; then echo "  ✅ $1（含 '$3'）"; PASS=$((PASS+1)); else echo "  ❌ $1 — 输出未见 '$3'"; sed -n '1,5p' "$2"; FAIL=$((FAIL+1)); fi; }
chk_not(){ if grep -q -- "$3" "$2"; then echo "  ❌ $1 — 不应出现 '$3'"; FAIL=$((FAIL+1)); else echo "  ✅ $1（无 '$3'）"; PASS=$((PASS+1)); fi; }
chk_ne(){ if [ "$2" != "$3" ]; then echo "  ✅ $1（$2 ≠ $3）"; PASS=$((PASS+1)); else echo "  ❌ $1 — rc=$2 不应等于 $3"; FAIL=$((FAIL+1)); fi; }

comp(){ "$PX" build "$1" "$2" >"$WORK/build.log" 2>&1 || { echo "  ❌ 编译失败 $2"; tail -8 "$WORK/build.log"; FAIL=$((FAIL+1)); return 1; }; }
# 每例在 $WORK/build/<name> 出产物

# ---------------- 用例 1：.has()/contains/get 正常语义（两轨逐字节一致） ----------------
cat > "$WORK/has.px" <<'PXI'
def main():
    d = {"a": 1, "b": 2}
    print(str(d.has("a")) + " " + str(d.has("z")) + " " + str(d.contains("b")))
    j = json_parse("{\"x\": 1}")
    print(str(j.has("x")) + " " + str(j.has("y")))
    print(str(d.get("zz", "def")))
PXI
echo "── 1) .has()/contains/get（严格口径的守卫手段本身必须可用）"
"$PX" run "$WORK/has.px" > "$WORK/has.interp.txt" 2>&1; ri=$?
comp --vm "$WORK/has.px" && { "$WORK/build/has" > "$WORK/has.vm.txt" 2>&1; rv=$?; } || rv=99
chk "解释轨 rc" "$ri" "0"
chk "VM 轨 rc" "$rv" "0"
chk "两轨输出一致" "$(diff -q "$WORK/has.interp.txt" "$WORK/has.vm.txt" >/dev/null 2>&1 && echo same || echo diff)" "same"
chk_has "守卫语义正确" "$WORK/has.interp.txt" "true false true"

# ---------------- 用例 2：缺键 = R1008（两轨同码、退出码非 0） ----------------
cat > "$WORK/missing.px" <<'PXI'
def main():
    d = {"a": 1}
    print(str(d["nope"]))
PXI
echo "── 2) 缺键 d[\"nope\"] → R1008（解释轨 / VM 轨 / C 轨）"
for mode in interp vm c; do
  case $mode in
    interp) "$PX" run "$WORK/missing.px" > "$WORK/missing.$mode.txt" 2>&1; rc=$? ;;
    vm)     comp --vm "$WORK/missing.px" && { "$WORK/build/missing" > "$WORK/missing.$mode.txt" 2>&1; rc=$?; } || rc=99 ;;
    c)      comp --c "$WORK/missing.px" && { "$WORK/build/missing" > "$WORK/missing.$mode.txt" 2>&1; rc=$?; } || rc=99 ;;
  esac
  chk_ne "$mode 轨退出码非 0" "$rc" "0"
  chk_has "$mode 轨报 R1008" "$WORK/missing.$mode.txt" "R1008"
done

# ---------------- 用例 3：null.has(k) → R1007（两轨同码；迁移时必须先判 null） ----------------
cat > "$WORK/nullhas.px" <<'PXI'
def main():
    n = null
    print(str(n.has("k")))
PXI
echo "── 3) null.has(\"k\") → R1007"
"$PX" run "$WORK/nullhas.px" > "$WORK/nullhas.interp.txt" 2>&1; ri=$?
comp --vm "$WORK/nullhas.px" && { "$WORK/build/nullhas" > "$WORK/nullhas.vm.txt" 2>&1; rv=$?; } || rv=99
chk_ne "解释轨退出码非 0" "$ri" "0"
chk_ne "VM 轨退出码非 0" "$rv" "0"
chk_has "解释轨报 R1007" "$WORK/nullhas.interp.txt" "R1007"
chk_has "VM 轨报 R1007" "$WORK/nullhas.vm.txt" "R1007"

# ---------------- 用例 4：非字符串键 d.has(1) 必须报错、绝不段错误（E4 回归） ----------------
cat > "$WORK/intkey.px" <<'PXI'
def main():
    d = {"a": 1}
    print(str(d.has(1)))
PXI
echo "── 4) d.has(1) → R1002（回归：此前编译轨 SIGSEGV rc=139）"
"$PX" run "$WORK/intkey.px" > "$WORK/intkey.interp.txt" 2>&1; ri=$?
comp --vm "$WORK/intkey.px" && { "$WORK/build/intkey" > "$WORK/intkey.vm.txt" 2>&1; rv=$?; } || rv=99
chk "解释轨 rc=1" "$ri" "1"
chk "VM 轨 rc=1（非 139）" "$rv" "1"
chk_has "解释轨报 R1002" "$WORK/intkey.interp.txt" "R1002"
chk_has "VM 轨报 R1002" "$WORK/intkey.vm.txt" "R1002"

# ---------------- 用例 5：协程内运行时错误 → 宿主继续，但退出码必须非 0（E1） ----------------
# 注：等待时长取 1.5s（> 协程错误发生的时刻）——「顶层先返回、协程错误随后才被观测到」
#     是调度时序窗口（0.3s 实测 rc 抖动），该窗口另由 runtime 的 atexit 兜底，
#     本门只断言确定性形态。
cat > "$WORK/coro.px" <<'PXI'
def worker():
    d = {"a": 1}
    print(str(d["nope"]))

def main():
    spawn worker()
    sleep(1.5)
    print("host-alive")
PXI
echo "── 5) 协程内 R1008 → 宿主继续 + rc=1（此前 rc=0 = 假成功）"
comp --vm "$WORK/coro.px" && { "$WORK/build/coro" > "$WORK/coro.vm.txt" 2>&1; rv=$?; } || rv=99
chk "VM 轨 rc=1" "$rv" "1"
chk_has "宿主存活" "$WORK/coro.vm.txt" "host-alive"
chk_has "隔离消息在" "$WORK/coro.vm.txt" "已隔离"
comp --c "$WORK/coro.px" && { "$WORK/build/coro" > "$WORK/coro.c.txt" 2>&1; rc=$?; } || rc=99
chk "C 轨 rc=1" "$rc" "1"
chk_has "C 轨宿主存活" "$WORK/coro.c.txt" "host-alive"

# ---------------- 用例 6：PX_SPAWN_ISOLATE=0 逃生舱（E2） ----------------
echo "── 6) PX_SPAWN_ISOLATE=0 → 立即终止（不再隔离），两轨都要生效"
comp --vm "$WORK/coro.px" && { PX_SPAWN_ISOLATE=0 "$WORK/build/coro" > "$WORK/iso0.vm.txt" 2>&1; rv=$?; } || rv=99
chk_ne "VM 轨退出码非 0" "$rv" "0"
chk_not "VM 轨不再打印隔离消息" "$WORK/iso0.vm.txt" "已隔离"
chk_not "VM 轨宿主未继续" "$WORK/iso0.vm.txt" "host-alive"
comp --c "$WORK/coro.px" && { PX_SPAWN_ISOLATE=0 "$WORK/build/coro" > "$WORK/iso0.c.txt" 2>&1; rc=$?; } || rc=99
chk_ne "C 轨退出码非 0" "$rc" "0"
chk_not "C 轨不再打印隔离消息" "$WORK/iso0.c.txt" "已隔离"

# ---------------- 用例 7：handler 出错 → 客户端 500，且**不**把进程退出码拉成失败（消费语义） ----------------
echo "── 7) 服务端 handler 出错 → 500（M116 D7 不回归）+ 进程退出码仍为 0（消费语义）"
cat > "$WORK/serve.px" <<'PXI'
def handler(req):
    d = {"a": 1}
    print(str(d["nope"]))
    return "never"

def main():
    spawn http_serve_unix("/tmp/m120_srv.sock", handler)
    sleep(1.0)
    let r = http_unix("/tmp/m120_srv.sock", "/boom", "GET")
    print("client_status=" + str(r["status"]))
    sleep(0.3)
PXI
comp --vm "$WORK/serve.px" && { "$WORK/build/serve" > "$WORK/serve.vm.txt" 2>&1; rv=$?; } || rv=99
chk "VM 轨客户端收到 500" "$(grep -c 'client_status=500' "$WORK/serve.vm.txt")" "1"
chk "VM 轨进程退出码 0（handler 错误已被消费）" "$rv" "0"
chk_has "隔离消息仍在（可观测）" "$WORK/serve.vm.txt" "已隔离"

echo "──────────────────────────────"
echo "M120 门：PASS=$PASS FAIL=$FAIL（工作目录 $WORK）"
[ "$FAIL" = "0" ] || exit 1
exit 0
