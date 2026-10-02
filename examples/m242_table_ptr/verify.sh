#!/bin/bash
# M242 门 · **表元素指针「跨锁外泄」审计**（缺陷 403–405）
#
# 主题：`g_conns` / `g_hpend` / `g_pxpend` 三张表都是 `xrealloc` **增长**的 fd 索引表
#   —— 容量不够时**整体搬家**（数组 >128KB 时 glibc 走 mmap ⇒ 旧块被 munmap）。
#   ⇒ 任何**跨锁持有**的元素指针都不可信：读它 SEGV，写它**静默堆破坏**。
#   M240 缺陷 399 收口了 `g_pxpend`（被一次真实崩溃逼出来的），但「还有没有别的表？」
#   此前**无人能回答** —— 因为没有判据。
#
# 本门把那个问题判据化（三形状派生 + 契约表双向 + 持锁核验）：
#   形状 1 `return &g_X[...]`                       —— 6 处（3 张会搬家的表 + 3 张固定表）
#   形状 2 `return <accessor>(...)`                 —— 0 处
#   形状 3 `T* v = <accessor>(...); … return v;`    —— ⚠️ **首版扫描器看不见的形状**
#         实测 1 处：`px_evc_acquire` 把 `&g_conns[fd]` 交给调用方（= 邀请它锁外解引用）
#
# 修法（缺陷 403）：`px_evc_acquire` 改返回 **int**（0/-1）。两个调用点本就把返回值
#   **只当布尔**用 ⇒ 行为**完全等价**，但从此不可能被误用（形状 3 归零）。
#   ⚠️ 同时照出两个**判据自身的 bug**（判据诚实度）：
#     · 锁名不得由表名推断 —— `g_conns` 的锁叫 **`g_conn_mu`**（少个 s）⇒ 首版 11 项假阳
#     · 分支内早退的 `unlock` 不算解锁 —— `if (…) { unlock; return -1; }` ⇒ 1 项假阳
#
# 判据层：[1] 审计器自证 · [2] 实跑 0 违例 + 规模锚点 · [3] 契约表双向 · [4] 缺陷 403 收口在位
#        [5] 动态回归（并发连接）· [6][7] 负控 A/B · [8] 负控 C（判据自伤）· [9] 覆盖边界
# CI 用 `--neg-skip`。
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D="$ROOT/examples/m242_table_ptr"
AUD="$ROOT/selfhost/table_ptr_audit.py"
CT="$D/CONTRACT.tsv"
W=${M242_W:-/tmp/m242_gate}
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
PY=python3
jq_() { $PY -c "import json;d=json.load(open('$W/audit.json'));print($1)"; }

# 负控备份 + trap 兜底还原（中途被杀也不会把补丁留在源码里 —— M239s1 的教训）
cp -f "$ROOT/runtime/runtime.c" "$W/runtime.c.bak"
cp -f "$AUD" "$W/audit.py.bak"
cp -f "$CT" "$W/contract.bak"
restore() {
    cp -f "$W/runtime.c.bak" "$ROOT/runtime/runtime.c" 2>/dev/null
    cp -f "$W/audit.py.bak" "$AUD" 2>/dev/null
    cp -f "$W/contract.bak" "$CT" 2>/dev/null
    rm -rf "$D/build" 2>/dev/null      # M207 教训：语料在自己目录里跑 ⇒ 编译产物会落在仓库
}
trap restore EXIT
FP0=$(cat "$ROOT/runtime/runtime.c" "$AUD" "$CT" 2>/dev/null | sha256sum | cut -c1-16)

echo "══ [1] 审计器自证（含反向判据） ══"
$PY "$AUD" --self-test > "$W/st.log" 2>&1; rc=$?
chk "self-test rc=0" "[ $rc -eq 0 ]"
chk "self-test 标记" "grep -q TABLE-PTR-SELFTEST-OK $W/st.log"
chk "自证锚点数 ≥13" "grep -qE '自证 1[3-9]/' $W/st.log"

echo ""
echo "══ [2] 审计器实跑：0 违例 + 规模锚点 ══"
$PY "$AUD" --root "$ROOT" --json > "$W/audit.json" 2>"$W/audit.err"; arc=$?
chk "实跑 rc=0（无违例）" "[ $arc -eq 0 ]"
MV=$(jq_ "len(d['movable'])"); AC=$(jq_ "len(d['accessor'])")
CS=$(jq_ "d['checked_call_sites']"); LK=$(jq_ "len(d['leak23'])")
TABLELIST=$(jq_ "','.join(sorted(d['movable']))")
chk "规模锚点 MOVABLE≥6（实测 $MV）" "[ $MV -ge 6 ]"
chk "规模锚点 ACCESSOR≥5（实测 $AC）" "[ $AC -ge 5 ]"
chk "规模锚点 核验调用点≥20（实测 $CS）" "[ $CS -ge 20 ]"
chk "会搬家的表含 g_pxpend" "grep -q g_pxpend <<< '$TABLELIST'"
chk "会搬家的表含 g_hpend" "grep -q g_hpend <<< '$TABLELIST'"
chk "会搬家的表含 g_conns" "grep -q g_conns <<< '$TABLELIST'"

echo ""
echo "══ [3] 契约表（双向 + 锁名显式登记） ══"
chk "契约表 3 行（非注释）" "[ $(grep -vc '^#' "$CT") -eq 3 ]"
chk "锁名 g_conn_mu 显式登记（不得由表名推断）" "grep -q 'g_conn_mu' $CT"
chk "锁名 g_pxpend_mu" "grep -q 'g_pxpend_mu' $CT"
chk "锁名 g_hpend_mu" "grep -q 'g_hpend_mu' $CT"
chk "无违例（漏登记/过期/表名不符/锁名不存在 均由实跑覆盖）" "[ $arc -eq 0 ]"

echo ""
echo "══ [4] 缺陷 403 收口在位 ══"
chk "px_evc_acquire 返回 int" "grep -qF 'static int px_evc_acquire(int fd, int kind) {' $ROOT/runtime/runtime.c"
chk "已无返回表指针的 px_evc_acquire" "! grep -qF 'static PxConnCtx* px_evc_acquire' $ROOT/runtime/runtime.c"
chk "前向声明已改 int" "grep -qF 'static int px_evc_acquire(int fd, int kind);' $ROOT/runtime/runtime.c"
chk "调用点 20480 只判 0" "grep -qF 'if (px_evc_acquire(fd, kind_tmo) == 0)' $ROOT/runtime/runtime.c"
chk "调用点 21110 只判 0" "grep -qF 'if (px_evc_acquire(fd, FSERVE_KIND_SSE) == 0)' $ROOT/runtime/runtime.c"
chk "非 Linux stub 返回 -1" "grep -qF 'static int px_evc_acquire(int fd, int kind) { (void)fd; (void)kind; return -1; }' $ROOT/runtime/runtime.c"
chk "转发型外泄归零（实测 $LK）" "[ $LK -eq 0 ]"

echo ""
echo "══ [5] 动态回归：并发连接（最小 smoke） ══"
mkdir -p "$W/www"; echo hello-m242 > "$W/www/x.txt"
PORT=$(( 21400 + (RANDOM % 400) ))
$ROOT/tools/px build "$D/srv.px" -o "$W/srv" > "$W/build.log" 2>&1; brc=$?
chk "srv 编译 rc=0" "[ $brc -eq 0 ]"
# ⚠️ `tools/px build` 的产物落**源码目录**的 `build/`（`-o` 不生效）——
#    实测：`编译成功: …/examples/m242_table_ptr/build/srv`。按实际路径取。
SRVBIN="$D/build/srv"
if [ $brc -eq 0 ]; then
    M242_PORT=$PORT M242_WWW="$W/www" "$SRVBIN" > "$W/srv.log" 2>&1 &
    SRV=$!
    ready=0
    for i in $(seq 1 50); do
        if curl -s -o /dev/null --max-time 1 "http://127.0.0.1:$PORT/x.txt"; then ready=1; break; fi
        sleep 0.1
    done
    chk "服务端就绪（轮询就绪，非固定 sleep）" "[ $ready -eq 1 ]"
    if [ $ready -eq 1 ]; then
        seq 1 16 | xargs -P 16 -I{} curl -s -o /dev/null -w '%{http_code}\n' --max-time 5 \
            "http://127.0.0.1:$PORT/x.txt" > "$W/codes.txt" 2>&1
        OKN=$(grep -c '^200$' "$W/codes.txt" || true)
        chk "16 并发全 200（实测 $OKN）" "[ $OKN -eq 16 ]"
    fi
    kill "$SRV" 2>/dev/null; wait "$SRV" 2>/dev/null
fi
rm -rf "$D/build" 2>/dev/null          # 清理编译副作用（产品目录不落产物）
chk "深度覆盖由既有门承担（登记）" "true"

echo ""
if [ "$NEG" = 1 ]; then
    echo "══ [6] 负控 A：锁外取表指针 ⇒ 审计器必判红 ══"
    $PY - "$ROOT" <<'PYEOF'
import sys, io
p = sys.argv[1] + '/runtime/runtime.c'
s = io.open(p, encoding='utf-8').read()
old = ('    pthread_mutex_lock(&g_conn_mu);\n'
       '    PxConnCtx* c = px_evc_ctx(fd);\n'
       '    int n = (c && c->fd == fd) ? c->pbuf_len : 0;\n')
new = ('    PxConnCtx* c = px_evc_ctx(fd);   /* NEGCTL-M242A: 锁外取表指针 */\n'
       '    pthread_mutex_lock(&g_conn_mu);\n'
       '    int n = (c && c->fd == fd) ? c->pbuf_len : 0;\n')
assert s.count(old) == 1, 'NEG-A 锚点不唯一: %d' % s.count(old)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
print('  [neg-A] 已注入：px_conn_pend_len 锁外取 px_evc_ctx')
PYEOF
    $PY "$AUD" --root "$ROOT" > "$W/negA.log" 2>&1; ra=$?
    chk "负控 A 判红（rc=1）" "[ $ra -eq 1 ]"
    chk "负控 A 指名 px_conn_pend_len" "grep -q 'px_conn_pend_len' $W/negA.log"
    chk "负控 A 指名 未持锁" "grep -q '未持锁' $W/negA.log"
    restore

    echo ""
    echo "══ [7] 负控 B：契约表删一行 ⇒ 必判红（未登记契约） ══"
    grep -v '^g_hpend' "$CT" > "$W/ct_b.tsv"
    $PY "$AUD" --root "$ROOT" --contract "$W/ct_b.tsv" > "$W/negB.log" 2>&1; rb=$?
    chk "负控 B 判红（rc=1）" "[ $rb -eq 1 ]"
    chk "负控 B 指名 未登记契约" "grep -q '未登记契约' $W/negB.log"
    chk "负控 B 指名 http_pend_ctx" "grep -q 'http_pend_ctx' $W/negB.log"
    # 登记过期（反向）：表里多一行
    { cat "$CT"; echo 'g_nonexist_tbl	px_nonexist_ctx	locked-only	g_nonexist_mu	构造的过期行'; } > "$W/ct_c.tsv"
    $PY "$AUD" --root "$ROOT" --contract "$W/ct_c.tsv" > "$W/negB2.log" 2>&1; rb2=$?
    chk "负控 B2 登记过期判红（rc=1）" "[ $rb2 -eq 1 ]"
    chk "负控 B2 指名 登记过期" "grep -q '登记过期' $W/negB2.log"

    echo ""
    echo "══ [8] 负控 C：判据自伤 ⇒ 负控 A 的红必须消失 ══"
    $PY - "$AUD" <<'PYEOF'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = '    lk, uk = (RX_LOCK % mu), (RX_UNLOCK % mu)\n'
new = '    lk, uk = (RX_LOCK % mu), (RX_UNLOCK % mu)\n    return True   # NEGCTL-M242C: 判据自伤\n'
assert s.count(old) == 1, 'NEG-C 锚点不唯一: %d' % s.count(old)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
print('  [neg-C] 已注入：holding_mutex 恒真')
PYEOF
    # 负控 A 的形状重新注入（restore 后源码已还原）
    $PY - "$ROOT" <<'PYEOF'
import sys, io
p = sys.argv[1] + '/runtime/runtime.c'
s = io.open(p, encoding='utf-8').read()
old = ('    pthread_mutex_lock(&g_conn_mu);\n'
       '    PxConnCtx* c = px_evc_ctx(fd);\n'
       '    int n = (c && c->fd == fd) ? c->pbuf_len : 0;\n')
new = ('    PxConnCtx* c = px_evc_ctx(fd);   /* NEGCTL-M242A: 锁外取表指针 */\n'
       '    pthread_mutex_lock(&g_conn_mu);\n'
       '    int n = (c && c->fd == fd) ? c->pbuf_len : 0;\n')
assert s.count(old) == 1
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PYEOF
    $PY "$AUD" --root "$ROOT" > "$W/negC.log" 2>&1; rc2=$?
    chk "负控 C：A 的红消失（rc=0）" "[ $rc2 -eq 0 ]"
    restore
else
    echo "══ [6][7][8] 负控已跳过（--neg-skip） ══"
fi

echo ""
echo "══ [9] 覆盖边界 ══"
echo "  · 形状 2（\`return accessor(…)\`）本仓当前 0 处 ⇒ **未经实测**（判据在位，无样本）"
echo "  · \`null-only\` 契约档本表为空 ⇒ 「调用方只准判空」这条核验**未实现**（留待有样本时）"
echo "  · 动态层是**最小 smoke**（16 并发单请求）；keep-alive / 事件循环 / WS / SSE 的深度覆盖"
echo "    由既有门承担（m173 / m176 / m180 / m235 / m236）—— 本门不重复"

FP1=$(cat "$ROOT/runtime/runtime.c" "$AUD" "$CT" 2>/dev/null | sha256sum | cut -c1-16)
echo ""
echo "══ 门内未改动自检 ══"
chk "源码指纹与进门一致（无负控残留）" "[ '$FP0' = '$FP1' ]"

echo ""
echo "PASS=$pass FAIL=$fail"
[ "$fail" -eq 0 ] && echo "M242-VERIFY-OK" || echo "M242-VERIFY-FAIL"
exit $([ "$fail" -eq 0 ] && echo 0 || echo 1)
