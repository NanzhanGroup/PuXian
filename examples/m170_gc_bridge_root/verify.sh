#!/usr/bin/env bash
# ============================================================
# M170 门（第 55 轮）：native 桥的 **precise GC 根面** 收口 —— 缺陷 187（+ 同族）+ 缺陷 188
# ------------------------------------------------------------
# 背景（清歌 2026-09-21 报障 · 本门即其最小复现的工程化）：
#   `sqlite_query` 返回的行 dict 在 **GC 第一次回收后**被破坏 —— 列表元素 type 变
#   `unknown`/`int`、行 dict 丢键（R1002/R1008）。实测（M169 树）：
#     · 默认阈值 ⇒ `FAIL-TYPE n=16388 i=0 t=unknown`（3/3 次一致）
#     · `PX_GC_THRESHOLD=1000` ⇒ 更早发作（n≈23）
#     · GC 关（`PX_GC_THRESHOLD=1e8 PX_GC_TRIGGER_BYTES=0`）⇒ 15 万轮零破坏
#   ⇒ 病灶在**运行期 GC**，不在 .px 源码。
# 根因：VM 轨产物默认 **precise GC**（`px_gc_set_precise(1)`，不扫整条 C 栈），根面 =
#   全局槽 + VM 帧槽 + TLS 登记根栈（`px_root_push`/`PX_KEEP`）。native 桥（`bi_*`）里
#   只活在 **C 局部** 的 LXValue 若未登记，就在「另一次分配触发 GC」时被误回收 ⇒ 返回的
#   容器里是**已释放对象**（写坏 slab 空闲链 ⇒ 后续 SIGSEGV）。
#   本门锁定的两条**硬约束**（缺一即红）：
#     ① 容器创建后**必须登记**；② 登记必须**紧跟创建、先于下一次分配**
#        （写成 `d=px_dict(); hdr=px_dict(); PX_KEEP(d);` 时 hdr 那次分配就可能回收 d）。
#   C 轨（逃生舱）与解释轨不启用 precise ⇒ 三轨对照正是判据之一。
# 缺陷 188（本轮同族新登记）：`px_error` 的 longjmp **不展开 C 帧** ⇒ 隔离点落点处被
#   跳过的登记会成为**野根**（指向已释放对象，之后每轮 GC 都去标记它）。修法 = 隔离点
#   setjmp 前记录深度、落点 `px_root_restore` 收缩（本门第 ⑤ 层用 `PX_GC_DEBUG=1` 的
#   `root 还原` 行做**可观测**断言）。
# 判据（逐层可单独变红）：
#   ① 三轨一致：解释轨 / VM 轨（默认）/ C 轨 跑 `sqlite_root.px` ⇒ 同一条 `pass=.. fail=0`
#   ② VM 低阈值（`PX_GC_THRESHOLD=800`，即报障触发条件）× **长跑** ⇒ 零破坏
#   ③ stress 层（`PX_GC_STRESS=1`：**每次分配即 GC**）⇒ sqlite/xml/onnx/rsa 全绿
#      —— 这一层把「靠阈值凑巧发作」变成「必然发作」，是本门的主检测器
#   ④ 反例对照：`PX_GC_STRESS=1` 下 **C 轨**同样绿（证明差异只在精确根面，不是数据问题）
#   ⑤ H3/QPACK codec 值完整性：stress 下静态/动态两路 6 条头逐条一致（缺陷 190）
#   ⑥ 缺陷 188：隔离点 `root 还原` 行数 == 被隔离的错误数
#   ⑦ http_request 响应头完整性（stress：**缺陷 191** —— `h_exchange` 里响应头 dict 未登记）
#   ⑧ 负控 4 道（默认跑，`--neg-skip` 跳过）—— 各自独立判红 + sha256 逐字节还原：
#      A sqlite：注释 `PX_KEEP(out)`（恢复缺陷 187）⇒ ② 层必红
#      B xml：把登记顺序改回「先建两个 dict 再登记」⇒ ③ 层必红（SIGSEGV）
#      C runtime：隔离点 restore 换 `if (0)`（恢复缺陷 188）⇒ ⑥ 层必红
#      D qpack：去掉解码 `val` 的 PX_KEEP（恢复缺陷 190）⇒ ⑤ 层必红
#   ⚠️ **收尾修订（同轮内发现并修掉的自伤）**：缺陷 188 的第一版实现有两处硬伤，
#      导致 m120_dict_strict 门「serve handler 出错」用例 **18/30 次 SIGSEGV**（单跑门偶发红）：
#        ① 记录值用**栈局部**（`int rd_marks = 0; int rd_roots = px_root_depth(&rd_marks);`）
#           再在 longjmp 落点读取 ⇒ C 标准不保证有效 ⇒ 实测拿到 `marks_depth = -1811936416`
#           ⇒ `g_px_root_marks_n` 被设成负数 ⇒ 下一次 `px_root_push` 写 `marks[负数]` ⇒ 崩；
#        ② 归还时**收缩了 marks 栈** ⇒ marks 栈是线程级 TLS，而执行流是协程级
#           （M93 帧协程 M:N：同线程多协程交替/迁移）⇒ 弹掉其它协程的条目 ⇒ pop 弹错 mark。
#      修法 = 隔离点专用 API（`px_root_iso_mark` / `px_root_restore_iso`）：记录进 TLS
#        （哨兵 -1 = 本线程无记录即不动作）、归还**只收缩根栈**。实测：修前 18/30 SIGSEGV
#        → 修后 **0/30**（同一用例、同一判据）。本门 ⑥ 层与负控 C 随之改用新 API。
#   ⚠️ **191 为什么不配负控**（按纪律：**不设假负控**）：实测去掉 `PX_KEEP(*out_headers)` 后
#      **症状是时序相关的** —— 同源码连跑 5 次：1 次丢头 `R1008`、3 次 PASS、1 次 SIGSEGV
#      ⇒ 这类控制会让门**偶发变红**（假红比不判更糟）。故 191 只由 ⑦ 层**正判据锁症状**
#      （修前该样例在 stress 下 core dump；正向复现 2/2 独立运行）。
#   ⚠️ 另记**缺陷 192** —— **已由 M182 收口**（第 60 轮 · 见 `examples/m182_hdr_root/` 与
#      spec §17.10 第 3 条硬约束）。原文保留：`PX_GC_STRESS=1 PX_GC_INLINE=1`（两个调试
#      开关同开）下 m23c 必丢头（A 组含 KEEP 3/3 红、B 组去 KEEP 3/3 红 ⇒ **病因不在 h_exchange**）。
#      默认 / 低阈值 / 单开 stress 全绿；**M182 定位到真形状 = 调用方的「登记迟到」窗口**
#      （`LXValue d = px_dict();` 先于 `PX_KEEP(headers)`），修后**双开 3/3 绿**。
# 用法：./examples/m170_gc_bridge_root/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m170_gc_bridge_root/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT"
export LC_ALL=C LANG=C

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PXI="${PX_PXI_BIN:-./bootstrap/pxi}"
MODEL="$ROOT/examples/m156_onnx/models/t01_basic.onnx"
SRC_SQLITE="runtime/runtime_sqlite.c"
SRC_XML="runtime/runtime_xml.c"
SRC_RT="runtime/runtime.c"
SRC_QPACK="runtime/runtime_h3_qpack.c"
WORK="$(mktemp -d /tmp/m170.XXXXXX)"
DB="$WORK/m170.db"

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

snapshot() { for f in "$SRC_SQLITE" "$SRC_XML" "$SRC_RT" "$SRC_QPACK"; do cp "$f" "$WORK/$(basename "$f").bak"; done; }
restore_all() {
    for f in "$SRC_SQLITE" "$SRC_XML" "$SRC_RT" "$SRC_QPACK"; do
        b="$WORK/$(basename "$f").bak"
        [ -f "$b" ] && cp "$b" "$f"
    done
}
trap 'restore_all; rm -rf "$WORK"' EXIT

# 源指纹（负控前后必须逐字节一致）
src_sha() { for f in "$SRC_SQLITE" "$SRC_XML" "$SRC_RT" "$SRC_QPACK"; do sha256sum "$f"; done | sha256sum | cut -c1-16; }
SHA0="$(src_sha)"
snapshot

# 用例落盘：替换 __DB__ / __N__ / __MODEL__
mk() { # $1=源 $2=N $3=tag → $WORK/<tag>.px
    sed -e "s|__DB__|$DB|g" -e "s|__N__|$2|g" -e "s|__MODEL__|$MODEL|g" "$HERE/$1" > "$WORK/$3.px"
}
# VM 轨（默认）构建+跑：$1=tag $2=额外环境（可空）→ 输出落 $WORK/<tag>.vm.out，回显 rc
run_vm() {
    local tag=$1 env=${2:-}
    rm -rf "$WORK/build"
    if ! timeout 900 ./tools/px build "$WORK/$tag.px" > "$WORK/$tag.build.log" 2>&1; then
        echo "BUILDFAIL"; return 1
    fi
    env $env timeout 300 "$WORK/build/$tag" > "$WORK/$tag.vm.out" 2>&1
    echo $?
}
# C 轨（逃生舱）
run_c() {
    local tag=$1 env=${2:-}
    rm -rf "$WORK/build"
    if ! PX_BUILD_ENGINE=c timeout 900 ./tools/px build "$WORK/$tag.px" > "$WORK/$tag.cbuild.log" 2>&1; then
        echo "BUILDFAIL"; return 1
    fi
    env $env timeout 300 "$WORK/build/$tag" > "$WORK/$tag.c.out" 2>&1
    echo $?
}
# 解释轨（pxi）
run_interp() {
    local tag=$1 env=${2:-}
    env $env timeout 300 "$PXI" "$WORK/$tag.px" > "$WORK/$tag.interp.out" 2>&1
    echo $?
}

hdr "⓪ 夹具：自建 sqlite 库（gen_db.px · 不依赖任何生产文件）"
mk gen_db.px 0 t_gen_db
if ! timeout 900 ./tools/px build "$WORK/t_gen_db.px" > "$WORK/t_gen_db.build.log" 2>&1; then
    bad "夹具构建失败"
    tail -3 "$WORK/t_gen_db.build.log" | sed 's/^/      /'
else
    if ! timeout 300 "$WORK/build/t_gen_db" > "$WORK/t_gen_db.out" 2>&1; then
        bad "夹具运行失败"
        tail -3 "$WORK/t_gen_db.out" | sed 's/^/      /'
    fi
    grep -q '^fixture pending=20$' "$WORK/t_gen_db.out" \
        || { bad "夹具未就绪（应 fixture pending=20）"; tail -3 "$WORK/t_gen_db.out" | sed 's/^/      /'; }
    [ -s "$DB" ] || bad "夹具库文件为空：$DB"
fi

hdr "① 三轨一致：sqlite_query 行 dict 完整性（200 轮 × 20 行）"
mk sqlite_root.px 200 t_sqlite
rc_i=$(run_interp t_sqlite); rc_v=$(run_vm t_sqlite); rc_c=$(run_c t_sqlite)
for pair in "interp:$rc_i" "vm:$rc_v" "c:$rc_c"; do
    k=${pair%%:*}; rc=${pair##*:}
    if [ "$rc" != "0" ]; then bad "轨 $k rc=$rc（应为 0）"; tail -2 "$WORK/t_sqlite.$k.out" | sed 's/^/      /'; fi
    grep -q '^pass=4000 fail=0$' "$WORK/t_sqlite.$k.out" || { bad "轨 $k 未达 pass=4000 fail=0"; tail -2 "$WORK/t_sqlite.$k.out" | sed 's/^/      /'; }
done
if ! cmp -s "$WORK/t_sqlite.interp.out" "$WORK/t_sqlite.vm.out" || ! cmp -s "$WORK/t_sqlite.interp.out" "$WORK/t_sqlite.c.out"; then
    bad "三轨 stdout 不一致"
fi
note "三轨 pass=4000 fail=0 且逐字节一致"

hdr "② VM 低阈值长跑（PX_GC_THRESHOLD=800 · 报障触发条件 · 20000 轮）"
mk sqlite_root.px 20000 t_sqlite_long
rc=$(run_vm t_sqlite_long "PX_GC_THRESHOLD=800")
if [ "$rc" != "0" ]; then bad "低阈值长跑 rc=$rc（应为 0）"; tail -2 "$WORK/t_sqlite_long.vm.out" | sed 's/^/      /'; fi
grep -q '^pass=400000 fail=0$' "$WORK/t_sqlite_long.vm.out" || { bad "低阈值长跑未达 pass=400000 fail=0"; tail -2 "$WORK/t_sqlite_long.vm.out" | sed 's/^/      /'; }
note "pass=400000 fail=0"

hdr "③ stress 层（PX_GC_STRESS=1：每次分配即 GC）"
mk sqlite_root.px 3 t_s_sqlite;   mk xml_root.px 60 t_s_xml
mk onnx_root.px 3 t_s_onnx;       mk rsa_root.px 1 t_s_rsa
for spec in "t_s_sqlite:pass=60 fail=0" "t_s_xml:pass=60 fail=0" "t_s_onnx:pass=3 fail=0" "t_s_rsa:pass=1 fail=0"; do
    tag=${spec%%:*}; want=${spec##*:}
    rc=$(run_vm "$tag" "PX_GC_STRESS=1")
    if [ "$rc" != "0" ]; then bad "stress $tag rc=$rc（应为 0）"; tail -2 "$WORK/$tag.vm.out" | sed 's/^/      /'; continue; fi
    grep -q "^$want\$" "$WORK/$tag.vm.out" || { bad "stress $tag 未达 $want"; tail -2 "$WORK/$tag.vm.out" | sed 's/^/      /'; }
done

hdr "④ 反例对照：同一 stress 环境的 C 轨（保守 GC）"
rc=$(run_c t_s_sqlite "PX_GC_STRESS=1")
[ "$rc" = "0" ] || bad "C 轨 stress rc=$rc（应为 0）"
grep -q '^pass=60 fail=0$' "$WORK/t_s_sqlite.c.out" || bad "C 轨 stress 未达 pass=60 fail=0"
note "C 轨绿 ⇒ 差异只在精确根面"

hdr "⑤ H3/QPACK codec 值完整性（缺陷 190 · stress 层：每次分配即 GC）"
mk h3_codec_root.px 0 t_h3c
rc=$(run_vm t_h3c "PX_GC_STRESS=1")
if [ "$rc" != "0" ]; then bad "h3 codec rc=$rc（应为 0）"; tail -3 "$WORK/t_h3c.vm.out" | sed 's/^/      /'; fi
grep -q '^h3c-done static=6 dyn=3$' "$WORK/t_h3c.vm.out" \
    || { bad "h3 codec 未达 h3c-done static=6 dyn=3"; tail -3 "$WORK/t_h3c.vm.out" | sed 's/^/      /'; }
note "静态 6 条 + 动态 3 轮逐条一致（:authority/:path/user-agent 的值不得退化为名字）"

hdr "⑥ 缺陷 188：隔离点必须归还根登记深度（PX_GC_DEBUG=1）"
mk isolate_root.px 5 t_iso
rc=$(run_vm t_iso "PX_GC_DEBUG=1")
# ⚠️ 本层**不**对 rc=0 断言：PuXian 设计上「有协程因运行时错误被隔离终止 ⇒ 进程退出码
#   修正为 1」（见 [px-coro] 行）—— 这不是缺陷。本层的判据是**可观测的归还记录**。
grep -q '^spawned=5$' "$WORK/t_iso.vm.out" || bad "isolate 未完成 5 次 spawn"
grep -q 'root 还原' "$WORK/t_iso.vm.out" || bad "isolate 无任何 root 还原记录（缺陷 188 未修）"
n_restore=$(grep -c 'root 还原' "$WORK/t_iso.vm.out" || true)
if [ "${n_restore:-0}" -lt 5 ]; then
    bad "隔离点还原记录 $n_restore 条（应 ≥5 —— 每次被隔离的错误各一条）"
else
    note "隔离点还原记录 $n_restore 条（≥5 ⇒ 无野根残留）· rc=$rc（1 = 退出码修正，符合设计）"
fi
# 归还必须真的把深度**收缩**（roots=N→0 形态），只打印不收缩 = 假修
grep -qE 'root 还原 marks=[0-9]+→[0-9]+ roots=[0-9]+→0' "$WORK/t_iso.vm.out" \
    || bad "还原记录里没有 roots=…→0（说明只打印未收缩）"


hdr "⑦ http_request 响应头完整性（缺陷 191 · stress 层）"
# 为什么放这里：`sqlite_query` 与 `http_request` 是**同一类**缺陷的两个用户可见面
#   （桥里只活在 C 局部的容器未登记）。本层用仓库自带、**自包含**的样例（自己 spawn 服务端
#   + 客户端断言），在 stress 下跑 —— 修前：**core dump**（丢头 + 写已释放内存 ⇒ xmalloc 崩）。
m23="${ROOT}/examples/build/m23c_http_adv"
M23_SKIP=0
# ⚠️ 判据写法：`cmd | grep -c` 在**无匹配**时退出码为 1，`$(...) || echo 0` 会把「grep 的 0」
#   和「echo 的 0」**都**收进来（PORT_BUSY="0\n0"）⇒ 与 "0" 比较必不相等 ⇒ 层被误跳过（假跳过）。
PORT_BUSY=0
if command -v ss >/dev/null 2>&1 && ss -ltn 2>/dev/null | grep -q ':8899 '; then PORT_BUSY=1; fi
if [ "$PORT_BUSY" != "0" ]; then
    note "跳过：端口 8899 被占用（本层需要它；不是失败）"
    M23_SKIP=1
else
    if timeout 900 ./tools/px build examples/m23c_http_adv.px > "$WORK/m23c.build.log" 2>&1; then
        PX_GC_STRESS=1 timeout 300 "$m23" > "$WORK/m23c.stress.out" 2>&1
        rc_m23=$?
        if [ "$rc_m23" != "0" ]; then bad "http stress rc=$rc_m23（应为 0）"; tail -3 "$WORK/m23c.stress.out" | sed 's/^/      /'; fi
        grep -q 'HTTP-ADV TESTS PASSED' "$WORK/m23c.stress.out" \
            || { bad "http stress 未达 HTTP-ADV TESTS PASSED"; tail -3 "$WORK/m23c.stress.out" | sed 's/^/      /'; }
        [ "$rc_m23" = "0" ] && grep -q 'HTTP-ADV TESTS PASSED' "$WORK/m23c.stress.out" \
            && note "响应头完整（含自定义头 X-Test）· 无 SIGSEGV"
    else
        bad "m23c_http_adv 构建失败"
        tail -3 "$WORK/m23c.build.log" | sed 's/^/      /'
    fi
fi

# ---------------- 负控 ----------------
if [ "$NEG_SKIP" = "1" ]; then
    echo "（--neg-skip：跳过负控）"
else
    hdr "⑦ 负控 A：撤 sqlite 的 out 登记 + 令 g_tmp_root 离开 out ⇒ ② 层必红"
    # ⚠️ M209 两处修（M208 的改动打到本门）：
    #   ① 锚点随「S13 全仓收口」从 `PX_KEEP(out);` 变为 `px_root_push_keep(out);`
    #      （否则 replace 命中 0 次 = **静默 no-op** ⇒ 负控变成「什么都没改」⇒ 假红）。
    #      现在断言**唯一性**，命中数 != 1 即响亮失败。
    #   ② 形态换成「撤登记 + pad」：缺陷 269 的修复（回收出口不清 g_tmp_root）会**吸收**
    #      单纯的「撤登记」（out 就是最近登记的对象 ⇒ g_tmp_root 一直指着它）
    #      ⇒ 单纯撤登记已**不可判红**（M183/M208 同款教训：新修复吸收旧负控）。
    restore_all; snapshot
    python3 - "$SRC_SQLITE" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
old="    px_root_push_keep(out);\n"
new='    px_str("negctl-pad");   /* NEG-A：撤登记 + 令 g_tmp_root 离开 out */\n'
assert s.count(old) == 1, "NEG-A 锚点唯一性（实际 %d 次）" % s.count(old)
open(p,'w').write(s.replace(old,new,1))
PY
    rc=$(run_vm t_sqlite_long "PX_GC_THRESHOLD=800")
    if [ "$rc" = "0" ] && grep -q '^pass=400000 fail=0$' "$WORK/t_sqlite_long.vm.out"; then
        bad "负控 A 未判红（撤 out 登记 + pad 后仍全绿 ⇒ 判据没牙）"
    else
        note "负控 A 判红：rc=$rc · $(grep -m1 -E 'FAIL-(TYPE|KEY)' "$WORK/t_sqlite_long.vm.out" || echo '(无 FAIL 行)')"
    fi
    restore_all
    [ "$(src_sha)" = "$SHA0" ] || bad "负控 A 还原后源指纹不符"
    snapshot

    hdr "⑦ 负控 B：xml 登记顺序改回「先建两个 dict」⇒ ③ 层必红"
    python3 - "$SRC_XML" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
old="""    LXValue node = px_dict();
    px_root_push_keep(node);
    LXValue attrs = px_dict();
    PX_KEEP(attrs);"""
new="""    LXValue node = px_dict();
    LXValue attrs = px_dict();   /* NEG-B: 登记滞后 */
    px_root_push_keep(node);
    PX_KEEP(attrs);"""
assert old in s, "NEG-B 锚点未命中"
open(p,'w').write(s.replace(old,new,1))
PY
    rc=$(run_vm t_s_xml "PX_GC_STRESS=1")
    if [ "$rc" = "0" ] && grep -q '^pass=60 fail=0$' "$WORK/t_s_xml.vm.out"; then
        bad "负控 B 未判红（登记滞后仍全绿）"
    else
        note "负控 B 判红：rc=$rc"
    fi
    restore_all
    [ "$(src_sha)" = "$SHA0" ] || bad "负控 B 还原后源指纹不符"
    snapshot

    hdr "⑦ 负控 C：隔离点 restore 换 if(0) ⇒ ⑥ 层必红"
    python3 - "$SRC_RT" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
n=s.count("px_root_restore_iso();")
assert n >= 5, "NEG-C 锚点数 %d (<5)" % n
# ⚠️ 锚点 = **隔离点专用** API（px_root_restore_iso）。
#   h3 桥的「正常出口归一」用另一个接口（px_root_restore(rd_roots, rd_marks)）—— 那是**正常
#   返回路径**、不经 longjmp，关掉它会顺带改 h3 语义 ⇒ 不能当锚点（锚点必须只命中被考对象）。
s=s.replace("px_root_restore_iso();",
            "if (0) px_root_restore_iso();   /* NEG-C */")
open(p,'w').write(s)
print("NEG-C 命中 %d 处" % n)
PY
    rc=$(run_vm t_iso "PX_GC_DEBUG=1")
    n_restore2=$(grep -c 'root 还原' "$WORK/t_iso.vm.out" || true)
    # ⚠️ 判据只看**还原记录条数**，不看 rc —— isolate 用例的 rc 恒为 1（退出码修正，见 ⑥ 层），
    #    把 rc 写进判据会让本负控**恒判红**（假红 = 门自己坏掉，比不判还危险）。
    if [ "${n_restore2:-0}" -ge 5 ]; then
        bad "负控 C 未判红（restore 关闭后仍见 ≥5 条还原记录）"
    else
        note "负控 C 判红：还原记录 $n_restore2 条（<5）· rc=$rc"
    fi
    restore_all
    [ "$(src_sha)" = "$SHA0" ] || bad "负控 C 还原后源指纹不符"

    hdr "⑦ 负控 D：去掉 QPACK 解码 val 的登记（恢复缺陷 190）⇒ ⑤ 层必红"
    restore_all; snapshot
    python3 - "$SRC_QPACK" <<'PY'
import re, sys
p=sys.argv[1]; s=open(p).read()
# ⚠️ 必须**整行**替换（含行尾注释）—— 只替前缀会留下 `—— 下面 …` ⇒ C 编译失败，
#   门就成了「因为编译不过而判红」的**假红**（读不出真因，等于没判）。
pat=r"(?m)^            PX_KEEP\(val\);   // M170：\*\*必须紧跟创建\*\*.*$"
assert re.search(pat, s), "NEG-D 锚点未命中"
open(p,'w').write(re.sub(pat, "            /* PX_KEEP(val); NEG-D */", s, count=1))
PY
    rc=$(run_vm t_h3c "PX_GC_STRESS=1")
    if [ "$rc" = "0" ] && grep -q '^h3c-done static=6 dyn=3$' "$WORK/t_h3c.vm.out"; then
        bad "负控 D 未判红（去掉 val 登记仍全绿）"
    else
        note "负控 D 判红：rc=$rc · $(grep -m1 'FAIL-' "$WORK/t_h3c.vm.out" || echo '(无 FAIL 行)')"
    fi
    restore_all
    [ "$(src_sha)" = "$SHA0" ] || bad "负控 D 还原后源指纹不符"

fi

echo
if [ "$fail" = "0" ]; then
    echo "M170-VERIFY-OK"
else
    echo "M170-VERIFY-FAIL"
fi
exit $fail
