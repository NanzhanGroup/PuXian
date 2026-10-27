#!/usr/bin/env bash
# M120 · 字典缺键「严格」口径 + 协程错误不再静默（qg-issue 76）
# 覆盖：E1 协程隔离后退出码 / E2 PX_SPAWN_ISOLATE 逃生舱 / E3 错误码缺 R1007 / E4 非字符串键段错误
# 轨：解释器（px run）· VM 轨（px build 默认）· C 轨（px build --c）
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="$(mktemp -d /tmp/m120_gate.XXXXXX)"
# 第 21 轮：补 trap（同 m122 —— 本门原先每跑一次就把 build/ 产物留在 /tmp 不回收）
trap 'rm -rf "$WORK"' EXIT
PASS=0; FAIL=0
chk(){ if [ "$2" = "$3" ]; then echo "  ✅ $1"; PASS=$((PASS+1)); else echo "  ❌ $1 — 期望[$3] 实得[$2]"; FAIL=$((FAIL+1)); fi; }
chk_has(){ if grep -q -- "$3" "$2"; then echo "  ✅ $1（含 '$3'）"; PASS=$((PASS+1)); else echo "  ❌ $1 — 输出未见 '$3'"; sed -n '1,5p' "$2"; FAIL=$((FAIL+1)); fi; }
chk_not(){ if grep -q -- "$3" "$2"; then echo "  ❌ $1 — 不应出现 '$3'"; FAIL=$((FAIL+1)); else echo "  ✅ $1（无 '$3'）"; PASS=$((PASS+1)); fi; }
chk_ne(){ if [ "$2" != "$3" ]; then echo "  ✅ $1（$2 ≠ $3）"; PASS=$((PASS+1)); else echo "  ❌ $1 — rc=$2 不应等于 $3"; FAIL=$((FAIL+1)); fi; }
# 缺陷 588（M335）：把「偶发」变成「可量化」—— 一组重复运行里**任一次**等于 $2 就判红
chk_all_ne(){   # $1=名称 $2=多行 "i:rc" $3=禁止值 $4=K
    local bad total
    bad=$(printf '%s\n' "$2" | awk -F: -v z="$3" '$2==z{c++} END{print c+0}')
    total=$(printf '%s\n' "$2" | grep -c . )
    if [ "$bad" = 0 ]; then
        echo "  ✅ $1（$total 次全部 ≠ $3：$(printf '%s' "$2" | tr '\n' ' ')）"; PASS=$((PASS+1))
    else
        echo "  ❌ $1 — $bad/$total 次等于 $3（$(printf '%s' "$2" | tr '\n' ' ')）"; FAIL=$((FAIL+1))
    fi
}

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
# 注（M344 · 缺陷 600）：等待窗口曾写 `sleep(1.5)` 并注释成「1.5 秒」，**单位错**——
#     sleep 的单位是**毫秒**（同门用例 7 已注明，M304 实测 sleep(100)≈104ms），
#     即真实窗口只有 1.5ms。子线程从 pthread_create 到跑进协程体要 6~32ms（本机实测
#     进程总寿命上限），窗口比它小一个数量级 ⇒ 主线程先走、协程根本没启动 ⇒ 没有错误
#     可报 ⇒ rc=0 完全合法。这就是缺陷 588 记了两轮的「C 轨偶发 rc=0」的真根因。
#     修法：窗口 200ms（≥ 抖动上限 6 倍；300/300 实测稳定）+ **每轮断言前提「协程真的启动了」**
#     （台账口径 4：前提断言先于症状断言 —— 协程没跑起来时 rc=0 是空转，不是产品行为）。
cat > "$WORK/coro.px" <<'PXI'
def worker():
    print("worker-start")
    d = {"a": 1}
    print(str(d["nope"]))

def main():
    spawn worker()
    sleep(200)
    print("host-alive")
PXI
echo "── 5) 协程内 R1008 → 宿主继续 + rc=1（此前 rc=0 = 假成功）"
comp --vm "$WORK/coro.px" && { "$WORK/build/coro" > "$WORK/coro.vm.txt" 2>&1; rv=$?; } || rv=99
chk "VM 轨 rc=1" "$rv" "1"
chk_has "宿主存活" "$WORK/coro.vm.txt" "host-alive"
chk_has "隔离消息在" "$WORK/coro.vm.txt" "已隔离"
chk_has "VM 轨协程已启动（前提）" "$WORK/coro.vm.txt" "worker-start"
comp --c "$WORK/coro.px" && { "$WORK/build/coro" > "$WORK/coro.c.txt" 2>&1; rc=$?; } || rc=99
chk "C 轨 rc=1" "$rc" "1"
chk_has "C 轨宿主存活" "$WORK/coro.c.txt" "host-alive"
chk_has "C 轨协程已启动（前提）" "$WORK/coro.c.txt" "worker-start"

# ---------------- 用例 6：PX_SPAWN_ISOLATE=0 逃生舱（E2） ----------------
# M335（缺陷 588）：C 轨在本门**偶发** rc=0。实测频次：2026-10-26 全量门（4 分片 + 串行尾）
#   出现 1 次，随后同树单跑 1 次 + 连跑 10 次全绿 ⇒ 11 次里 1 次。
#   单次运行既不能定性也不能定频，所以本用例改为 **重复 K 次 + 每次留证**：
#     · 任一次 rc=0 ⇒ 判红（chk_all_ne）；
#     · 该次的完整输出与退出码复制到 $EVID（**不做 /tmp 清理**，下次发生时可直接看现场）。
#   为什么不是「重试到绿」：重试会把偶发掩盖掉。这里要的是把它量化，不是让它消失。
# M344（缺陷 600）**根因已证**：窗口 `sleep(1.5)` 是 1.5**毫秒**（不是秒），而子线程启动要
#   6~32ms ⇒ 主线程先退出、协程未启动。实验（本机 --c 产物，PX_SPAWN_ISOLATE=0）：
#     窗口 1.5ms ⇒ 300 次里 16 次 rc=0（5.3%，全量门并发下更糟）
#     窗口 200ms ⇒ 300 次 **0** 次 rc=0
#   失败的 16 次里输出只有 "host-alive"（或 "worker-start" 后就截断），stderr 全空 ⇒
#   **不是错误被吞**，是协程还没跑到 px_index 进程就退出了。
#   ⇒ 判据本身没错（rc 必须非 0），错的是它给的窗口太窄 + 没有前提断言。
K=${M120_ISO_REPEAT:-5}
EVID_BASE=/tmp/m120_iso_flake
iso0_run(){   # $1=轨道(vm|c) $2=输出前缀 ⇒ stdout 为每行 "i:rc"
    local track="$1" pref="$2" i rc f
    for i in $(seq 1 "$K"); do
        comp "--$track" "$WORK/coro.px" || { echo "$i:99"; continue; }
        f="$pref.$i.txt"
        ( PX_SPAWN_ISOLATE=0 "$WORK/build/coro" > "$f" 2>&1 ); rc=$?
        printf '%s:%s\n' "$i" "$rc"
        if [ "$rc" = 0 ]; then
            mkdir -p "$EVID_BASE" 2>/dev/null && {
                cp "$f" "$EVID_BASE/$(basename "$pref").$i.txt" 2>/dev/null
                printf 'track=%s iter=%s rc=0 at=%s\n' "$track" "$i" "$(date -Is)" >> "$EVID_BASE/occurrences.log"
            }
        fi
    done
}
iso0_pre(){  # $1=输出前缀 $2=轨道名 ⇒ 缺前提的轮次列表（空 = K 轮都真的启动了协程）
    local pref="$1" i miss=""
    for i in $(seq 1 "$K"); do
        grep -q 'worker-start' "$pref.$i.txt" 2>/dev/null || miss="$miss $i"
    done
    printf '%s' "$miss"
}
echo "── 6) PX_SPAWN_ISOLATE=0 → 立即终止（不再隔离），两轨都要生效（重复 K=$K 次 · 缺陷 588 留证）"
VM_RCS=$(iso0_run vm "$WORK/iso0.vm")
C_RCS=$(iso0_run c "$WORK/iso0.c")
chk_all_ne "VM 轨退出码非 0" "$VM_RCS" "0" "$K"
chk_all_ne "C 轨退出码非 0"  "$C_RCS"  "0" "$K"
# M344（缺陷 600）：前提断言 —— 协程没启动时 rc=0 是空转，不许当产品行为上报
VM_MISS=$(iso0_pre "$WORK/iso0.vm"); C_MISS=$(iso0_pre "$WORK/iso0.c")
chk "VM 轨 K 轮协程都已启动（前提）" "${VM_MISS:-ok}" "ok"
chk "C 轨 K 轮协程都已启动（前提）" "${C_MISS:-ok}" "ok"
# 隔离消息 / 宿主继续这两条只在首轮产物上判（重复运行的产物同构，逐轮判只是放大噪声）
chk_not "VM 轨不再打印隔离消息" "$WORK/iso0.vm.1.txt" "已隔离"
chk_not "VM 轨宿主未继续" "$WORK/iso0.vm.1.txt" "host-alive"
chk_not "C 轨不再打印隔离消息" "$WORK/iso0.c.1.txt" "已隔离"
if [ -d "$EVID_BASE" ] && [ -s "$EVID_BASE/occurrences.log" ]; then
    echo "  ℹ️  偶发现场已留存：$EVID_BASE（$(wc -l < "$EVID_BASE/occurrences.log") 条）"
fi

# ---------------- 用例 7：handler 出错 → 客户端 500，且**不**把进程退出码拉成失败（消费语义） ----------------
echo "── 7) 服务端 handler 出错 → 500（M116 D7 不回归）+ 进程退出码仍为 0（消费语义）"
cat > "$WORK/serve.px" <<'PXI'
def handler(req):
    d = {"a": 1}
    print(str(d["nope"]))
    return "never"

def main():
    spawn http_serve_unix("/tmp/m120_srv.sock", handler)
    # ⚠️ M304（2026-10-12）：`sleep` 的单位是**毫秒**（实测 `sleep(100)` ≈ 104ms）。
    #    这里原本写 `sleep(1.0)` = **1 毫秒**（本想等 1 秒就绪）⇒ 负载高时会假红
    #    （m118 已在 4 路分片下实测复现同款）。⇒ 改成真的 1 秒。
    sleep(1000)
    let r = http_unix("/tmp/m120_srv.sock", "/boom", "GET")
    print("client_status=" + str(r["status"]))
    sleep(300)
PXI
comp --vm "$WORK/serve.px" && { "$WORK/build/serve" > "$WORK/serve.vm.txt" 2>&1; rv=$?; } || rv=99
chk "VM 轨客户端收到 500" "$(grep -c 'client_status=500' "$WORK/serve.vm.txt")" "1"
chk "VM 轨进程退出码 0（handler 错误已被消费）" "$rv" "0"
chk_has "隔离消息仍在（可观测）" "$WORK/serve.vm.txt" "已隔离"

echo "──────────────────────────────"
echo "M120 门：PASS=$PASS FAIL=$FAIL（工作目录 $WORK）"
[ "$FAIL" = "0" ] || exit 1
exit 0
