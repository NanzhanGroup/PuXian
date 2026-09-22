#!/usr/bin/env bash
# ============================================================
# M182 门（第 60 轮）：native 桥的**「登记窗口」**收口 —— 缺陷 192（+ 同族）
# ------------------------------------------------------------
# 背景（M170 遗留的诚实边界 · 本轮收口）：
#   M170 把「容器创建后必须登记」这条纪律立起来了，但留下了两个洞：
#     ① 缺陷 192 未修：`PX_GC_STRESS=1 PX_GC_INLINE=1` 下 m23c **必**丢响应头 X-Test；
#     ② 「桥根面逐一审计」没做 —— 结论只到「被 stress 筛出来的那些都绿」。
#
# **本轮定位到的真形状（缺陷 192 的根因）**：
#   ```c
#   if (h_exchange(&slot, …, &headers, …) == 0) {   // h_exchange 返回前已 px_root_pop() 自己的帧
#       if (keep_alive) hpool_put(key, slot);
#       LXValue d = px_dict();                      // ← 这一次分配就可能回收 headers
#       px_root_push(); PX_KEEP(headers); PX_KEEP(d);   // ← 登记**迟到**了
#   ```
#   `headers` 从「离开 h_exchange 的登记帧」到「被 PX_KEEP」之间**只由调用者的 C 局部持有**，
#   而 VM 轨 precise GC **不扫 C 栈** ⇒ 期间任何一次分配触发 GC 都会把它收走。
#   ⇒ **第一条硬约束要补一句**：登记作用域必须**先于**该值的第一个「跨分配窗口」建立，
#      而不只是「创建之后尽快」——「返回值/out-param」形态最容易踩（值的生命从**被调用方**
#      的帧里出来，调用方接手的那一瞬间是裸的）。
#
# **为什么长期只在调试开关组合下发作（这条是本轮最值钱的机制解释）**：
#   `px_alloc` 里 `deferrable = (g_active_threads > 0) && !g_gc_force_inline` —— 多线程服务
#   模式（spawn/连接池活跃）**默认把 GC 延迟到安全点**（ISSUE28-B1）；延迟期间窗口自然被
#   「稍后的登记」补上 ⇒ **靠运气遮住**。`PX_GC_INLINE=1` 强制内联回收 ⇒ 窗口必现（3/3）。
#   实测对照：基线绿 · 单开 STRESS 绿 · 单开 INLINE 绿 · **双开 3/3 红**。
#
# 判据（逐层可单独变红）：
#   ① `examples/m23c_http_adv.px` 在 `PX_GC_STRESS=1 PX_GC_INLINE=1` 下整门通过
#      （修前：`R1008 字典没有键 'X-Test'`，3/3 必现）
#   ② `probe.px`（json_path_set 三路 + **http_unix 成功路径**）基线 vs 压力档**逐字节一致**，
#      且 `PROBE-JSON bad=0` / `PROBE-UNIX bad=0`
#      —— ② 同时是本仓**第一个 `http_unix` 成功路径的门**（此前只有连接失败用例），
#         对端用 `unix_srv.py`（python3 · UDS 上的极简 HTTP 应答器）。
#   ③ 负控 3 道（默认跑；`--neg-skip` 跳过）—— 各自独立判红 + sha256 逐字节还原：
#      A bi_http_request：把登记挪回「响应 dict 之后」⇒ ① 必红（确定性 3/3）
#      B json_path_set_at（dict 分支）：同法 ⇒ ② 必红
#      C bi_http_unix：同法 ⇒ ② 必红
# 用法：./examples/m182_hdr_root/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m182_hdr_root/verify.sh --neg-skip （只跑正判据，CI 用）
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
WORK="$(mktemp -d /tmp/m182.XXXXXX)"
SOCK="$WORK/px-m182.sock"
fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

snapshot()   { cp "$SRC_RT" "$WORK/runtime.c.bak"; }
restore_all(){ [ -f "$WORK/runtime.c.bak" ] && cp "$WORK/runtime.c.bak" "$SRC_RT"; }
trap 'restore_all; rm -rf "$WORK"' EXIT
sha() { sha256sum "$SRC_RT" | cut -c1-16; }
SHA0="$(sha)"
snapshot

# 构建到 $WORK/build（$1=源 $2=tag）——失败回显尾部日志
build() {
    rm -rf "$WORK/build"
    if ! timeout 900 ./tools/px build "$1" > "$WORK/$2.build.log" 2>&1; then
        tail -3 "$WORK/$2.build.log" | sed 's/^/      /'
        return 1
    fi
}
sed "s|__SOCK__|$SOCK|g" "$HERE/probe.px" > "$WORK/probe.px"
cp "$ROOT/examples/m23c_http_adv.px" "$WORK/m23c_http_adv.px"

# ───────────────────────── ① m23c：HTTP 客户端响应头根面 ─────────────────────────
hdr "① m23c_http_adv 在压力档（STRESS+INLINE）下必须整门通过"
if build "$WORK/m23c_http_adv.px" m23c; then
    PX_GC_STRESS=1 PX_GC_INLINE=1 timeout 300 "$WORK/build/m23c_http_adv" \
        > "$WORK/m23c.s.out" 2>&1
    rc=$?
    if [ "$rc" != "0" ]; then
        bad "m23c 压力档 rc=$rc（应为 0）"
        tail -3 "$WORK/m23c.s.out" | sed 's/^/      /'
    elif ! grep -q 'HTTP-ADV TESTS PASSED' "$WORK/m23c.s.out"; then
        bad "m23c 压力档未达 HTTP-ADV TESTS PASSED"
        tail -3 "$WORK/m23c.s.out" | sed 's/^/      /'
    else
        note "m23c 压力档 PASS（修前该档 3/3 报 R1008 丢 X-Test）"
    fi
else
    bad "m23c_http_adv 构建失败"
fi

# ───────────────────────── ② 探针：json_path_set × http_unix ─────────────────────────
hdr "② 探针（json_path_set 三路 + http_unix 成功路径）：基线 vs 压力档逐字节一致"
python3 "$HERE/unix_srv.py" "$SOCK" > "$WORK/uds.log" 2>&1 &
UDS_PID=$!
for _ in $(seq 1 60); do [ -S "$SOCK" ] && break; sleep 0.1; done
if [ ! -S "$SOCK" ]; then
    bad "UDS 夹具未就绪（$SOCK）"
    tail -3 "$WORK/uds.log" | sed 's/^/      /'
elif build "$WORK/probe.px" probe; then
    timeout 300 "$WORK/build/probe" > "$WORK/probe.n.out" 2>&1; rn=$?
    PX_GC_STRESS=1 PX_GC_INLINE=1 timeout 600 "$WORK/build/probe" > "$WORK/probe.i.out" 2>&1; ri=$?
    if [ "$rn" != "0" ]; then
        bad "探针基线 rc=$rn（应为 0）"; tail -3 "$WORK/probe.n.out" | sed 's/^/      /'
    fi
    if [ "$ri" != "0" ]; then
        bad "探针压力档 rc=$ri（应为 0）"; tail -3 "$WORK/probe.i.out" | sed 's/^/      /'
    fi
    grep -q '^PROBE-JSON bad=0$' "$WORK/probe.n.out" \
        || { bad "基线 PROBE-JSON 非 bad=0"; tail -3 "$WORK/probe.n.out" | sed 's/^/      /'; }
    grep -q '^PROBE-UNIX bad=0$' "$WORK/probe.n.out" \
        || { bad "基线 PROBE-UNIX 非 bad=0（http_unix 头/体不符）"; tail -3 "$WORK/probe.n.out" | sed 's/^/      /'; }
    grep -q '^PROBE-JSON bad=0$' "$WORK/probe.i.out" \
        || { bad "压力档 PROBE-JSON 非 bad=0"; tail -3 "$WORK/probe.i.out" | sed 's/^/      /'; }
    grep -q '^PROBE-UNIX bad=0$' "$WORK/probe.i.out" \
        || { bad "压力档 PROBE-UNIX 非 bad=0（http_unix 头/体不符）"; tail -3 "$WORK/probe.i.out" | sed 's/^/      /'; }
    cmp -s "$WORK/probe.n.out" "$WORK/probe.i.out" \
        || { bad "压力档与基线输出不一致"; diff "$WORK/probe.n.out" "$WORK/probe.i.out" | head -6 | sed 's/^/      /'; }
    note "探针 基线：$(tr '\n' ' ' < "$WORK/probe.n.out")"
    note "探针 压力档：$(tr '\n' ' ' < "$WORK/probe.i.out")"
else
    bad "探针构建失败"
fi
kill "$UDS_PID" 2>/dev/null
wait "$UDS_PID" 2>/dev/null

# ───────────────────────── ③ 负控 ─────────────────────────
# 纪律：每道负控前先 restore_all() 再 snapshot()（各自独立、干净起点，M165 教训）
negcase() {   # $1=标题 $2=python 补丁 $3=目标源 $4=tag $5=运行环境 $6=输出文件 $7=必须出现的绿判据
    local title="$1" patch="$2" src="$3" tag="$4" env="$5" outf="$6" green="$7"
    restore_all; snapshot
    if ! python3 -c "$patch"; then bad "$title：补丁锚点未命中"; return; fi
    if ! build "$src" "$tag"; then bad "$title：构建失败"; restore_all; return; fi
    env $env timeout 300 "$WORK/build/$(basename "${src%.px}")" > "$outf" 2>&1
    local rc=$?
    local red=0
    [ "$rc" != "0" ] && red=1
    grep -q "$green" "$outf" 2>/dev/null || red=1
    if [ "$red" = "1" ]; then
        note "$title ⇒ 判红 ✓（rc=$rc）"
    else
        bad "$title ⇒ **未判红**（负控失效：该处并非本门覆盖的根因）"
    fi
    restore_all
}
PATCH_A='import io,sys
p="runtime/runtime.c"; s=open(p,encoding="utf-8").read()
new="""            px_root_push();
            PX_KEEP(headers);   // headers 已离开 h_exchange 的登记帧 ⇒ 必须立刻接住
            if (keep_alive) hpool_put(key, slot);
            else { if (slot.tls) https_close(slot.tls); close(slot.fd); }
            LXValue d = px_dict();
            PX_KEEP(d);   // 紧跟创建（中间不得插入任何可能分配的调用）"""
old="""            if (keep_alive) hpool_put(key, slot);
            else { if (slot.tls) https_close(slot.tls); close(slot.fd); }
            LXValue d = px_dict();
            px_root_push();
            PX_KEEP(headers);
            PX_KEEP(d);"""
assert new in s, "anchor A"
open(p,"w",encoding="utf-8").write(s.replace(new,old)); print("PATCH-A-OK")'
PATCH_B='p="runtime/runtime.c"; s=open(p,encoding="utf-8").read()
new="""        px_root_push();
        LXValue d = (base.type == PX_DICT) ? base : px_dict();
        PX_KEEP(d);   // base（bi_json_path_set 深拷贝临时，仅 C 持有）跨拷贝分配存活
        LXObject* o = d.as.obj;
        LXValue r = px_dict();
        PX_KEEP(r);   // 结果 dict 跨 px_dict_set/json_value_copy 分配"""
old="""        LXValue d = (base.type == PX_DICT) ? base : px_dict();
        LXObject* o = d.as.obj;
        LXValue r = px_dict();
        px_root_push();
        PX_KEEP(d);   // base（bi_json_path_set 深拷贝临时，仅 C 持有）跨拷贝分配存活
        PX_KEEP(r);   // 结果 dict 跨 px_dict_set/json_value_copy 分配"""
assert new in s, "anchor B"
open(p,"w",encoding="utf-8").write(s.replace(new,old)); print("PATCH-B-OK")'
PATCH_C='p="runtime/runtime.c"; s=open(p,encoding="utf-8").read()
new="""    px_root_push();
    PX_KEEP(headers);
    LXValue d = px_dict();
    PX_KEEP(d);   // 紧跟创建"""
old="""    LXValue d = px_dict();
    px_root_push();
    PX_KEEP(headers);
    PX_KEEP(d);"""
assert new in s, "anchor C"
open(p,"w",encoding="utf-8").write(s.replace(new,old)); print("PATCH-C-OK")'

if [ "$NEG_SKIP" = "1" ]; then
    hdr "③ 负控：--neg-skip（CI 用）⇒ 跳过"
else
    hdr "③ 负控 3 道（各自独立判红 + 源逐字节还原）"
    negcase "负控 A · bi_http_request 登记迟到（恢复缺陷 192）" "$PATCH_A" \
            "$WORK/m23c_http_adv.px" negA "PX_GC_STRESS=1 PX_GC_INLINE=1" \
            "$WORK/negA.out" "HTTP-ADV TESTS PASSED"
    python3 "$HERE/unix_srv.py" "$SOCK" > "$WORK/uds2.log" 2>&1 &
    UDS2=$!
    for _ in $(seq 1 60); do [ -S "$SOCK" ] && break; sleep 0.1; done
    negcase "负控 B · json_path_set_at（dict 分支）登记迟到" "$PATCH_B" \
            "$WORK/probe.px" negB "PX_GC_STRESS=1 PX_GC_INLINE=1" \
            "$WORK/negB.out" "^PROBE-JSON bad=0\$"
    negcase "负控 C · bi_http_unix 登记迟到" "$PATCH_C" \
            "$WORK/probe.px" negC "PX_GC_STRESS=1 PX_GC_INLINE=1" \
            "$WORK/negC.out" "^PROBE-UNIX bad=0\$"
    kill "$UDS2" 2>/dev/null
    wait "$UDS2" 2>/dev/null
fi

hdr "④ 源还原核对"
restore_all
SHA1="$(sha)"
if [ "$SHA0" = "$SHA1" ]; then
    note "runtime.c 逐字节还原 ✓（$SHA0）"
else
    bad "runtime.c 未逐字节还原：$SHA0 → $SHA1"
fi
grep -q 'M182（缺陷 192）' "$SRC_RT" || bad "M182 注释标记丢失（源可能被负控残留污染）"

echo
if [ "$fail" = "0" ]; then
    echo "M182-VERIFY-OK"
    exit 0
fi
echo "M182-VERIFY-FAIL"
exit 1
