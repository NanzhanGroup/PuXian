#!/bin/bash
# ============================================================
# M257 门 · **容器「存储已被回收/复用」的响亮自检** + 分配器一致性检查
# ------------------------------------------------------------
# 病灶（实测 2026-10-04）：`bi_udp_recv` 的容器 r 在仍在作用域内时其存储被 GC 回收并被紧接着的
#   分配复用（复用者 = `px_bytes_len(buf,4)` 的 bytes 对象）⇒ `px_dict_set` 扩容分支按「旧 dict」
#   解释失效内存 ⇒ `xfree` 收到垃圾指针（实测 0xffffffff00000004）⇒ 既不在 slab 也不像 mmap 块
#   ⇒ `*(p-8)` 读未映射页 ⇒ SIGSEGV。同日另有 2 份 core 是**下游**表现（`xmalloc ← px_root_push`
#   与 `xmalloc ← px_dict`，即空闲链表已被写坏 ⇒ 报错指不到根因）。
#
# 判据四层：
#   [1] 静态：四处修复在位（三处容器守卫 + xmalloc 一致性检查 + 根栈失衡自检）+ 行序
#   [2] 基线：小语料正常跑通（证明守卫**不误伤**合法路径）+ 计数入口已导出
#   [3] 注入正判据：把期望类型改错 ⇒ 守卫**必须**拦住并响亮报告、输出**必须**退化
#   [4] 负控：三处各自独立判红 + 源逐字节还原
# ⚠️ 覆盖边界（如实登记）：
#   · 守卫是**缓解不是根治** —— 它把「静默堆损坏」降级为「响亮 + 该次操作空转」，
#     真正的「存储为何在作用域内被回收」本轮**未定论**（见 CHANGELOG §未收口）。
#   · 复现器**对时序敏感**：单实例原探针 ~1/60 复现；加任何探测器/守卫后 60~240 次均未复现
#     ⇒ 本门**不把「跑 N 次不崩」当判据**（那会是假绿），只把「守卫可被注入触发且真的拒绝」当判据。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
SRC="runtime/runtime.c"
W="${M257_W:-$(mktemp -d /tmp/m257gate.XXXXXX)}"
SNAP="$W/snap_runtime.c"
NEG_SKIP="${NEG_SKIP:-0}"

snapshot()    { cp "$ROOT/$SRC" "$SNAP"; }
restore_all() { [ -f "$SNAP" ] && cp "$SNAP" "$ROOT/$SRC"; return 0; }
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS + 1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL + 1)); }

# 唯一性锚点替换（命中数 ≠ 1 ⇒ 响亮失败，绝不静默 no-op）
patch_one() {
    python3 - "$ROOT/$SRC" "$1" "$2" <<'PY'
import sys, io
p, old, new = sys.argv[1], sys.argv[2], sys.argv[3]
s = io.open(p, encoding='utf-8').read()
k = s.count(old)
if k != 1:
    sys.stderr.write("ANCHOR-BAD count=%d\n" % k); sys.exit(3)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
PY
}

rebuild() {   # $1=日志
    rm -rf "$HERE/build"
    ( cd "$ROOT" && ./tools/px build "$HERE/probe_dict.px" ) >"$1" 2>&1
}
run_probe() { # $1=日志 → 打印 rc
    ( cd "$ROOT" && timeout -k 5 60 "$HERE/build/probe_dict" ) >"$1" 2>&1
    echo "$?"
}
chk_grep() {  # $1=描述 $2=文件 $3=模式 $4=最少命中数
    local n
    n="$(grep -c -- "$3" "$ROOT/$2" 2>/dev/null || true)"
    [ -z "$n" ] && n=0
    if [ "$n" -ge "$4" ]; then ok "$1（命中 $n）"; else bad "$1（命中 $n < $4）"; fi
}

echo "══ M257 门：容器存储失效自检 + 分配器一致性 ══"

# ---------- [1] 静态 ----------
echo "── [1] 静态：四处修复在位"
chk_grep "容器守卫定义在位"         "$SRC" "static void px_ctr_guard_fail" 1
chk_grep "px_list_push 守卫"        "$SRC" 'px_ctr_guard_fail(o, PX_LIST, "px_list_push")' 1
chk_grep "px_dict_set 守卫"         "$SRC" 'px_ctr_guard_fail(o, PX_DICT, "px_dict_set")' 1
chk_grep "px_dict_get 守卫"         "$SRC" 'px_ctr_guard_fail(o, PX_DICT, "px_dict_get")' 1
chk_grep "xmalloc 一致性检查"       "$SRC" "SLAB BUG: bad-alloc" 1
chk_grep "xfree 一致性检查（对称）" "$SRC" "SLAB BUG: bad-free" 1
chk_grep "根栈失衡自检"             "$SRC" "M257-ROOT" 1

# 行序判据：守卫必须在 PX_UAFCHK 之后（先自检、再解引用）
order_ok=1
for pair in 'px_list_push(list)|px_ctr_guard_fail(o, PX_LIST, "px_list_push")' \
            'px_dict_set(dict)|px_ctr_guard_fail(o, PX_DICT, "px_dict_set")' \
            'px_dict_get(dict)|px_ctr_guard_fail(o, PX_DICT, "px_dict_get")'; do
    marc="${pair%%|*}"; gpat="${pair##*|}"
    ml="$(grep -n -F -- "PX_UAFCHK(o, \"$marc\")" "$ROOT/$SRC" | head -1 | cut -d: -f1)"
    gl="$(grep -n -F -- "$gpat" "$ROOT/$SRC" | head -1 | cut -d: -f1)"
    if [ -z "$ml" ] || [ -z "$gl" ] || [ "$gl" -le "$ml" ]; then
        order_ok=0; echo "    （$marc: UAFCHK@${ml:-?} 守卫@${gl:-?}）"
    fi
done
if [ "$order_ok" = "1" ]; then ok "三处守卫均在 PX_UAFCHK 之后（先自检再解引用）"
else bad "守卫行序（应在 PX_UAFCHK 之后）"; fi

# ---------- [2] 基线 ----------
echo "── [2] 基线：守卫不误伤合法路径"
if rebuild "$W/build_base.log"; then
    rc="$(run_probe "$W/out_base.txt")"
    if [ "$rc" = "0" ] && grep -q '^M257-BASE-OK$' "$W/out_base.txt" && grep -q '^DICT=4$' "$W/out_base.txt"; then
        ok "[2] 基线语料 rc=0 · DICT=4 · 跑到结尾（守卫未误伤）"
    else
        bad "[2] 基线语料异常 rc=$rc"; tail -5 "$W/out_base.txt"
    fi
else
    bad "[2] 基线编译失败"; tail -5 "$W/build_base.log"
fi
if command -v nm >/dev/null 2>&1 && nm -g "$HERE/build/probe_dict" 2>/dev/null | grep -q "px_ctr_guard_count"; then
    ok "[2] 计数入口 px_ctr_guard_count 已导出"
else
    bad "[2] 计数入口未导出"
fi

# ---------- [3][4] 注入正判据 + 负控 ----------
if [ "$NEG_SKIP" = "1" ]; then
    echo "── [3][4] --neg-skip（CI 档）⇒ 跳过注入与负控"
else
    echo "── [3] 注入正判据：期望类型 PX_DICT→PX_LIST ⇒ 必须响亮拒绝 + 输出退化"
    restore_all; snapshot
    if patch_one 'if (o->type != PX_DICT) { px_ctr_guard_fail(o, PX_DICT, "px_dict_set"); return; }' \
                 'if (o->type != PX_LIST) { px_ctr_guard_fail(o, PX_LIST, "px_dict_set"); return; }' \
       && rebuild "$W/build_inj.log"; then
        rc="$(run_probe "$W/out_inj.txt")"
        if grep -q 'M257-CTR' "$W/out_inj.txt"; then
            ok "[3] 守卫已被触发（打印 [M257-CTR] 报告，rc=$rc）"
        else
            bad "[3] 未打印 [M257-CTR]（守卫未被触发）"; tail -6 "$W/out_inj.txt"
        fi
        if ! grep -q '^DICT=4$' "$W/out_inj.txt"; then
            ok "[3] 输出已退化（字面量写不进去 ⇒ 拒绝生效，未按失效对象继续写）"
        else
            bad "[3] 输出未退化 ⇒ 守卫只报不改，仍按错误类型继续操作"
        fi
    else
        bad "[3] 注入补丁或重建失败"; tail -4 "$W/build_inj.log"
    fi

    echo "── [4] 负控 A：把 px_dict_set 守卫改成 if (0)"
    restore_all; snapshot
    # ⚠️ 替换后的文本必须**不再包含**被 grep 的原文，否则判据「撤掉了却仍命中」= 自伤假红
    #    （本门首版即踩：新文本里原样保留了同一调用串）。
    if patch_one 'if (o->type != PX_DICT) { px_ctr_guard_fail(o, PX_DICT, "px_dict_set"); return; }' \
                 'if (0) { px_ctr_guard_fail(o, PX_DICT, "px_dict_set_disabled"); return; }'; then
        n="$(grep -c -- 'px_ctr_guard_fail(o, PX_DICT, "px_dict_set")' "$ROOT/$SRC" || true)"
        if [ "$n" = "0" ]; then ok "[4A] 撤掉守卫 ⇒ 静态判据不再命中（有牙）"
        else bad "[4A] 撤掉守卫后静态判据仍命中（n=$n）"; fi
    else
        bad "[4A] 补丁失败"
    fi

    echo "── [4] 负控 B：删掉 xmalloc 一致性检查"
    restore_all; snapshot
    if patch_one 'SLAB BUG: bad-alloc' 'SLAB-NOOP bad-alloc'; then
        n="$(grep -c -- 'SLAB BUG: bad-alloc' "$ROOT/$SRC" || true)"
        if [ "$n" = "0" ]; then ok "[4B] 删掉 bad-alloc ⇒ 静态判据不再命中（有牙）"
        else bad "[4B] 删掉后仍命中（n=$n）"; fi
    else
        bad "[4B] 补丁失败"
    fi

    echo "── [4] 负控 C：删掉根栈失衡自检"
    restore_all; snapshot
    if patch_one 'M257-ROOT' 'M257-NOOP'; then
        n="$(grep -c -- 'M257-ROOT' "$ROOT/$SRC" || true)"
        if [ "$n" = "0" ]; then ok "[4C] 删掉失衡自检 ⇒ 静态判据不再命中（有牙）"
        else bad "[4C] 删掉后仍命中（n=$n）"; fi
    else
        bad "[4C] 补丁失败"
    fi
fi

# ---------- [5] 源还原 ----------
echo "── [5] 源逐字节还原"
restore_all
if cmp -s "$SNAP" "$ROOT/$SRC"; then ok "[5] runtime.c 与快照逐字节一致"
else bad "[5] runtime.c 未还原（负控残留）"; fi

echo
if [ "$FAIL" -eq 0 ]; then
    echo "M257-VERIFY-OK（通过 $PASS / 失败 $FAIL）"
    exit 0
fi
echo "M257-VERIFY-FAIL（通过 $PASS / 失败 $FAIL）"
exit 1
