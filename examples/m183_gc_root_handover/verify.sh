#!/usr/bin/env bash
# ============================================================
# M183 门（第 61 轮）：根栈「交棒窗口」+ native 桥漏登记 —— 缺陷 197 / 198
# ------------------------------------------------------------
# 两个缺陷同族（「GC 误回收仍在使用的对象」），但**形状不同**，本轮各修一处：
#
#  ① 缺陷 198（**写坏空闲链表** · SIGSEGV）：`bi_s3_list` 里
#     `LXValue l = px_list(0);` 之后**从未登记**，而循环里 `px_str(key)` 每次都分配
#     ⇒ precise GC（根面 = 全局槽 + VM 帧槽 + TLS 登记根栈，不扫 C 栈）在下一次分配
#     就地回收 `l` ⇒ `px_list_push` 往**已释放的 list** 里写元素 ⇒ 写坏 slab 空闲链表
#     首字 ⇒ 真凶与崩溃点相隔很远（实测崩在 `http_conn_worker` 的 `px_dict()` 里
#     `s->in_use[idx]` 越界读；`r13 = s->free_head = 4`）。
#     仅 `PX_GC_STRESS=1 PX_GC_INLINE=1` 必现（3/3），默认/单开开关均 0/3。
#
#  ② 缺陷 197（**读数静默错值** · 无崩溃）：native 桥的惯用法是
#     `px_root_push(); PX_KEEP(x); …; px_root_pop(); return x;`，而 **pop 的出口**
#     （`gc_unblock_stop` → `gc_pause_if_requested`，M110-S2 协作式安全点）是一个暂停点：
#     服务模式下由 fserve worker 在 `px_gc_poll()` 触发并发 GC ⇒ 本线程恰在 pop 处
#     被暂停，而**返回值 x 此刻既不在本帧（刚 pop）也不在调用方帧（尚未 PX_KEEP）**
#     ⇒ 只由调用方的 C 局部持有 ⇒ 被回收。实测（`PX_GC_STRESS=1`，5/6 复现）：
#     `bi_http_request` 的 `headers` 被回收、其槽随即被 `px_dict()` 复用
#     ⇒ `d.as.obj == headers.as.obj` ⇒ `d["headers"] = d`（自引用环）
#     ⇒ 客户端读响应头得到错值（`R1008 字典没有键 'X-M183'` / json "encountered a cycle"）。
#     注意这不是「登记迟到」而是「**登记交棒有缝**」——M182 把登记提到最前面仍不够，
#     因为缝在**被调用方的收帧**里。
#
# 修法（两条，各自可独立判红）：
#  · 198：`bi_s3_list` 补 `px_root_push(); PX_KEEP(l);`（并配对 pop）。
#  · 197：**根栈收缩延迟**——`px_root_pop()` 只记「待收缩深度」`g_px_trunc_pending`，
#    物理根条目留到调用方 `PX_KEEP`（= 接住动作）时才回收，且收缩点由**本帧逻辑基**钳制
#    （绝不删本帧自己的根）。⇒ 暂停窗口内物理根栈是「超集」：只多标、不少标（安全）；
#    全帧弹出后下一次 keep 即收缩到 0 ⇒ 物理栈有界（本门第 ④ 层即断言这条）。
#
# 判据（逐层可单独变红）：
#  ① handover.px（进程内 http_serve + http_request ×40）在 `PX_GC_STRESS=1` 下
#     **连跑 10 次全绿**（修前 1/6 通过）—— 这是 197 的直接判据。
#  ② handover.px + s3flow.px 在 `PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_UAFDET=1` 下
#     rc=0 且出现 `M37_S3_OK`，**且不出现 `PX_GC_UAFDET` 报错**（修前 s3list 3/3 SIGSEGV）。
#  ③ `PX_GC_TRACE=1` 下**不得**出现 `[PX_GC_TRACE]`（硬不变量：登记在根栈里的对象
#     绝不能被回收）—— 两个探针 × 两种压力档。
#  ④ `PX_GC_DEBUG=1` 的根栈峰值有界（所有线程 max roots ≤ 64）—— 防「延迟收缩」退化成泄漏。
#  ⑤ 负控 4 道（默认跑；`--neg-skip` 跳过）—— 各自独立判红 + 源逐字节还原：
#     A `px_root_pop` 退回「立即收缩」          ⇒ ① 必红（恢复缺陷 197）
#     B `bi_s3_list` 去掉 push/KEEP/pop         ⇒ ② 必红（恢复缺陷 198）
#     C `px_root_keep` 去掉「待收缩应用」        ⇒ ④ 必红（根栈无界增长）
#     D `bi_px_exec` 去掉 env/srv 登记           ⇒ ③b 必红（恢复缺陷 199，UAFDET 3/3）
#     ⚠️ M209 修（M208 的改动打到本门，第 9 次同族）：
#        · C 的锚点**不唯一**（同一段「待收缩应用」M208 后在 push_keep 里也有一份内联）
#          ⇒ 首版 `replace(…,1)` 只删**一处** ⇒ 泄漏消失不复现 ⇒ 负控**失去牙**。
#          修法 = 正则匹配 **全部 2 处**并断言「恰好 2 处」。
#        · D 的锚点把 `px_root_push_keep(env);` 换成 `LXValue env = px_dict();`
#          ⇒ 与上一行**重声明** ⇒ **编译失败** ⇒ chk_199 因构建失败而返回 1 ⇒
#          「判红」了，但**判红原因不是本缺陷**（假绿）。修法 = 正确删除该行 + 三处
#          锚点各加**唯一性断言**；并给 negcase 加「构建失败 ⇒ 不算判红」的守卫。
#
# ③b 缺陷 199（同族第三处 · 由本门的 UAFDET 筛出）：`bi_px_exec` 里 `env` / `srv` 两个
#   dict 裸 C 局部，后续 `px_dict()`×4 / `px_str` / `px_call(json_stringify)` 都会分配
#   ⇒ 被回收后 `px_dict_set` 写进已成空闲槽的 **keys 数组**（class 64）⇒ 写坏空闲链表。
#   实测 examples/m32_hot_reload：修前 3/3 UAFDET+core，修后 0/4（判据只看「不得报 UAFDET」，
#   不依赖 px_exec 子进程在该环境是否成功）。
# 用法：./examples/m183_gc_root_handover/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m183_gc_root_handover/verify.sh --neg-skip （只跑正判据，CI 用）
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
WORK="$(mktemp -d /tmp/m183.XXXXXX)"
fail=0
BUILD_FAIL=0   # M209：负控「判红」若因**构建失败**而来，不算判红（等于没验证）
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

snapshot()    { cp "$SRC_RT" "$WORK/runtime.c.bak"; }
restore_all() { [ -f "$WORK/runtime.c.bak" ] && cp "$WORK/runtime.c.bak" "$SRC_RT"; }
trap 'restore_all; rm -rf "$WORK"' EXIT
# 每道负控前先 restore_all 再 snapshot（各自独立、干净起点 —— M165 教训）
sha() { sha256sum "$SRC_RT" | cut -c1-16; }
SHA0="$(sha)"
snapshot

cp "$HERE/handover.px" "$WORK/handover.px"
cp "$HERE/s3flow.px"  "$WORK/s3flow.px"
cp "$ROOT/examples/m32_hot_reload.px" "$WORK/m32_hot_reload.px"

# 构建（$1=源 $2=tag）→ 隔离产物目录 $WORK/b<tag>/build/<name>
#   注意：每个探针**各自独立目录**（不能共用 $WORK/build —— 后一次构建会清掉前一次的产物，
#   负控重建时尤其危险）。失败回显尾部日志。
build() {
    local src="$1" tag="$2" d
    d="$WORK/b$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$src" "$d/"
    BUILD_FAIL=0
    if ! timeout 900 ./tools/px build "$d/$(basename "$src")" > "$WORK/$tag.build.log" 2>&1; then
        BUILD_FAIL=1
        tail -4 "$WORK/$tag.build.log" | sed 's/^/      /'
        return 1
    fi
    return 0
}
# 逐字节压力簇：$1=二进制 $2=成功标记 其余=环境
runs_ok() {   # $1=n $2=bin $3=标记 $4=env ... → 全部通过返回 0
    local n="$1" bin="$2" mark="$3"; shift 3
    local i
    for i in $(seq 1 "$n"); do
        env "$@" timeout 300 "$bin" > "$WORK/run.$i.out" 2>&1
        if ! grep -q "^$mark\$" "$WORK/run.$i.out"; then
            tail -2 "$WORK/run.$i.out" | sed 's/^/      /'
            return 1
        fi
    done
    return 0
}

BIN_HO="$WORK/bhandover/build/handover"
BIN_S3="$WORK/bs3flow/build/s3flow"

# ─────────────── 判定函数（返回 0 = 绿；负控复用它们）───────────────
chk_build_base() {   # 基线（无压力档）
    build "$WORK/handover.px" handover || return 1
    env timeout 300 "$BIN_HO" > "$WORK/base.out" 2>&1 || return 1
    grep -q '^HANDOVER OK$' "$WORK/base.out"
}
chk_197() {          # ① 197 直接判据：STRESS 下连跑 10 次（每次先重建：负控要靠它体现补丁）
    build "$WORK/handover.px" handover || return 1
    runs_ok 10 "$BIN_HO" "HANDOVER OK" PX_GC_STRESS=1
}
chk_198() {          # ② 198 直接判据（UAFDET 武装）
    build "$WORK/s3flow.px" s3flow || return 1
    env PX_GC_UAFDET=1 PX_GC_STRESS=1 PX_GC_INLINE=1 timeout 300 "$BIN_S3" > "$WORK/s3.out" 2>&1 || return 1
    grep -q 'PX_GC_UAFDET' "$WORK/s3.out" && return 1
    grep -q '^M37_S3_OK$' "$WORK/s3.out"
}
chk_199() {          # ③b 199 判据：px_exec 的 env/srv 登记（只判「不得报 UAFDET」，子进程成败不判）
    build "$WORK/m32_hot_reload.px" m32 || return 1
    env PX_GC_UAFDET=1 PX_GC_STRESS=1 PX_GC_INLINE=1 timeout 300 "$WORK/bm32/build/m32_hot_reload" > "$WORK/m32.out" 2>&1
    ! grep -q 'PX_GC_UAFDET' "$WORK/m32.out"
}
chk_handover_inline() {
    build "$WORK/handover.px" handover || return 1
    runs_ok 3 "$BIN_HO" "HANDOVER OK" PX_GC_UAFDET=1 PX_GC_STRESS=1 PX_GC_INLINE=1
}
chk_trace() {        # ③ 硬不变量：登记在根栈里的对象绝不能被回收
    local f
    for f in "$WORK/trace.ho.out" "$WORK/trace.s3.out"; do
        [ -f "$f" ] || return 1
        grep -q 'PX_GC_TRACE' "$f" && return 1
    done
    return 0
}
chk_peak() {         # ④ 根栈峰值有界（防延迟收缩退化成泄漏）
    build "$WORK/handover.px" handover || return 1
    env PX_GC_DEBUG=1 PX_GC_STRESS=1 timeout 300 "$BIN_HO" > "$WORK/peak.out" 2>&1 || return 1
    grep -q '^HANDOVER OK$' "$WORK/peak.out" || return 1
    local mx
    mx=$(grep -o 'roots=[0-9][0-9]*' "$WORK/peak.out" | cut -d= -f2 | sort -n | tail -1)
    [ -n "$mx" ] || return 1
    note "根栈峰值（全线程 max roots）= $mx"
    [ "$mx" -le 64 ]
}

# ───────────────────────── 正判据 ─────────────────────────
hdr "① 基线（无压力档）：handover.px 必须整门通过"
if chk_build_base; then
    note "基线：$(tr '\n' ' ' < "$WORK/base.out")"
else
    bad "基线失败"; tail -3 "$WORK/base.out" 2>/dev/null | sed 's/^/      /'
fi

hdr "② 缺陷 197：STRESS 下连跑 10 次（修前 1/6 通过）"
if [ -x "$BIN_HO" ]; then
    if chk_197; then note "10/10 通过 ✓"; else bad "197 复现：丢响应头（HANDOVER OK 未全绿）"; fi
    if chk_handover_inline; then note "STRESS+INLINE ×3 通过 ✓"; else bad "handover 压力档不过"; fi
else
    bad "handover 未构建"
fi

hdr "③ 缺陷 198：s3flow.px（= m37_s3 口径）在 STRESS+INLINE+UAFDET 下（修前 3/3 SIGSEGV）"
if chk_198; then
    note "s3flow：$(tr '\n' ' ' < "$WORK/s3.out")"
else
    bad "198 未过（崩溃/UAFDET 报错/标记缺失）"; tail -6 "$WORK/s3.out" 2>/dev/null | sed 's/^/      /'
fi

hdr "③b 缺陷 199：px_exec 的 env/srv 登记（m32_hot_reload 压力档不得报 UAFDET）"
if chk_199; then
    note "无 UAFDET 命中 ✓"
else
    bad "199 未过（桥局部漏登记：dict 的 keys 数组被写坏空闲链表）"
    grep -A3 'PX_GC_UAFDET' "$WORK/m32.out" 2>/dev/null | head -6 | sed 's/^/      /'
fi

hdr "④ 硬不变量：PX_GC_TRACE 不得报「回收了仍登记在根栈里的对象」"
env PX_GC_TRACE=1 PX_GC_STRESS=1 timeout 300 "$BIN_HO" > "$WORK/trace.ho.out" 2>&1
env PX_GC_TRACE=1 PX_GC_STRESS=1 PX_GC_INLINE=1 timeout 300 "$BIN_S3" > "$WORK/trace.s3.out" 2>&1
if chk_trace; then note "两探针 × 两档：无 PX_GC_TRACE 命中 ✓"; else
    bad "PX_GC_TRACE 命中（根面漏扫）"
    grep -A3 'PX_GC_TRACE' "$WORK/trace.ho.out" "$WORK/trace.s3.out" 2>/dev/null | head -8 | sed 's/^/      /'
fi

hdr "⑤ 根栈峰值有界（延迟收缩不得退化成泄漏）"
if chk_peak; then note "峰值 ≤ 64 ✓"; else bad "根栈峰值超限或运行失败"; fi

# ───────────────────────── 负控 ─────────────────────────
PATCH_A='p="runtime/runtime.c"; s=open(p,encoding="utf-8").read()
new="""    g_px_trunc_pending = mark;
    gc_unblock_stop(&old);"""
old="""    g_px_roots_n = mark;
    gc_unblock_stop(&old);"""
assert new in s, "anchor A"
open(p,"w",encoding="utf-8").write(s.replace(new,old,1)); print("PATCH-A-OK")'

PATCH_B='p="runtime/runtime.c"; s=open(p,encoding="utf-8").read()
n1="""    px_root_push_keep(l);
    if (st == 200) {"""
o1="""    if (st == 200) {"""
n2="""    free(body);
    px_root_pop();   // M183：与上方 px_root_push 配对（返回值由调用方 VM 槽接管）
    return l;"""
o2="""    free(body);
    return l;"""
assert n1 in s and n2 in s, "anchor B"
s=s.replace(n1,o1,1).replace(n2,o2,1)
open(p,"w",encoding="utf-8").write(s); print("PATCH-B-OK")'

PATCH_C='p="runtime/runtime.c"; s=open(p,encoding="utf-8").read()
import re
pat=re.compile(r"^[ \t]*if \(g_px_trunc_pending >= 0\) \{\n(?:[ \t]*int fbase[^\n]*\n)(?:[ \t]*int t = g_px_trunc_pending;\n)(?:[ \t]*if \(t < fbase\)[^\n]*\n)(?:[ \t]*if \(t < g_px_roots_n\)[^\n]*\n)(?:[ \t]*g_px_trunc_pending = -1;\n)[ \t]*\}\n", re.M)
n=len(pat.findall(s))
assert n==2, "anchor C 出现 %d 次（应 2 处：px_root_push_keep 内联 + px_root_keep 本体）" % n
open(p,"w",encoding="utf-8").write(pat.sub("", s)); print("PATCH-C-OK (2 处)")'

PATCH_D='p="runtime/runtime.c"; s=open(p,encoding="utf-8").read()
n1="""        LXValue env = px_dict();\n        px_root_push_keep(env);   // 紧跟创建（中间不得插入任何可能分配的调用）\n"""
o1="""        LXValue env = px_dict();\n"""
n2="""        LXValue srv = px_dict();\n        PX_KEEP(srv);"""
o2="""        LXValue srv = px_dict();"""
n3="""        px_root_pop();   // M183：与上方 px_root_push 配对\n"""
assert s.count(n1)==1, "anchor D1 唯一性（实际 %d）" % s.count(n1)
assert s.count(n2)==1, "anchor D2 唯一性（实际 %d）" % s.count(n2)
assert s.count(n3)==1, "anchor D3 唯一性（实际 %d）" % s.count(n3)
s=s.replace(n1,o1,1).replace(n2,o2,1).replace(n3,"",1)
open(p,"w",encoding="utf-8").write(s); print("PATCH-D-OK")'

negcase() {   # $1=标题 $2=python 补丁 $3=应判红的判定函数
    local title="$1" patch="$2" fn="$3"
    restore_all; snapshot; BUILD_FAIL=0
    if ! python3 -c "$patch"; then bad "$title：补丁锚点未命中"; restore_all; return; fi
    if "$fn"; then
        bad "$title ⇒ **未判红**（负控失效：该处并非本门覆盖的根因）"
    elif [ "$BUILD_FAIL" = "1" ]; then
        bad "$title ⇒ **构建失败**（判红原因不是本缺陷 ⇒ 负控无效，等于没验证）"
    else
        note "$title ⇒ 判红 ✓"
    fi
    restore_all
}

if [ "$NEG_SKIP" = "1" ]; then
    hdr "⑥ 负控：--neg-skip（CI 用）⇒ 跳过"
else
    hdr "⑥ 负控 3 道（各自独立判红 + 源逐字节还原）"
    negcase "负控 A · px_root_pop 退回「立即收缩」（恢复缺陷 197）" "$PATCH_A" chk_197
    negcase "负控 B · bi_s3_list 去掉 push/KEEP/pop（恢复缺陷 198）"  "$PATCH_B" chk_198
    negcase "负控 C · px_root_keep 去掉「待收缩应用」（根栈泄漏）"     "$PATCH_C" chk_peak
    negcase "负控 D · bi_px_exec 去掉 env/srv 登记（恢复缺陷 199）"    "$PATCH_D" chk_199
fi

hdr "⑦ 源还原核对"
restore_all
SHA1="$(sha)"
if [ "$SHA0" = "$SHA1" ]; then
    note "runtime.c 逐字节还原 ✓（$SHA0）"
else
    bad "runtime.c 未逐字节还原：$SHA0 → $SHA1"
fi
grep -q 'M183（缺陷 197）' "$SRC_RT" || bad "M183 注释标记丢失（源可能被负控残留污染）"

echo
if [ "$fail" = "0" ]; then
    echo "M183-VERIFY-OK"
    exit 0
fi
echo "M183-VERIFY-FAIL"
exit 1
