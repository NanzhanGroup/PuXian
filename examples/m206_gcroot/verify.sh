#!/usr/bin/env bash
# ============================================================
# M206 门（第 85 轮）：native 桥 **GC 根面漏登记** —— 静态审计器 + 压力差分 + 19 处收口
# ------------------------------------------------------------
# 主题：把「创建 → 未登记 → 再分配」这个**形状**全仓找齐（M170/182/183 修的都是
#   「被压力筛筛出来的」，没有一次是「静态扫全仓找齐」）。
#
# 判据（M170 的两条硬约束）：① 容器/对象创建后必须登记；② 登记必须**紧跟创建、
#   先于下一次分配**（下一次分配就可能触发 GC；precise 模式下根面 = 全局槽 +
#   VM 帧槽 + TLS 登记根栈，**不扫 C 栈**）。
#
# 层：
#  ① 静态：`selfhost/gcroot_audit.py` 自证（M209 起 **16 锚点**：8 必中 / 8 必不中，
#     含 hit6b/hit7/hit8 三条**反向判据** —— 证明排除规则没把工具改瞎）
#  ② 静态：全仓扫描**候选 ⇄ 已判定基线**（`BASELINE.tsv`）逐条对齐；
#     基线里**不得**存在「真」判定（= 未修的真缺陷）；规模下限（函数 ≥ 1500、登记站点 ≥ 200）
#     ⚠️ M209（第 88 轮）：本表**已收敛为 0 条** —— M206 那 8 条「人工判定」已**下沉为判据**
#        （触发点仅构造器 / 覆盖 / 作用域 / deref 交棒；见 `docs/GC_ROOTS.md` §9 与
#        `examples/m209_gcroot_rules/`），故「候选 ⇄ 基线」现在是**两空集**对齐；
#        本表自此是**安全网**（任何新增候选都必须进表）。
#  ③ 动态·HTTP 面：probe_rt.px（20 次 urlencoded + 20 次 multipart POST，含 session 族）
#     —— 基线档与**压力档**（PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1）都必须 40/40 OK，
#     且压力档**不得**出现 `PX_GC_LIVECHK`（修前：响亮 + SIGABRT/core）
#  ④ 动态·UDP 面：probe_udp.px（udp_serve 的 hargs 族）—— 同两档
#  ⑤ 负控 3 道（各自独立判红 + 源逐字节还原）：
#     A 撤 `bi_udp_recv` 的 KEEP(r)            ⇒ ④ 压力档必红（缺陷 258 复现，实测 LIVECHK+core）
#     B 撤 `px_session_read` 的 KEEP(v)        ⇒ ③ 压力档必红（缺陷 254 复现，实测 `json: 对象解析失败`）
#     C 撤 `h3_extra_to_headers` 的 KEEP(pair) ⇒ ② 静态必红（缺陷 253 复现）
#   ⚠️ 覆盖边界（如实登记）：`px_parse_urlenc` / `px_parse_cookie` / `px_quic_raw_*` 三族
#      由**静态判据**（层 ②）覆盖 —— http_serve 路径用的是**内联** urlencoded 解析（实测：
#      撤 `px_parse_urlenc` 的 KEEP 后 probe_rt 仍全绿 ⇒ 该探针不经过它），故不给它们配动态负控。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 9

NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

W="$(mktemp -d /tmp/m206_gate.XXXXXX)"
SNAP="$W/snap"
SRC_LIST="runtime/runtime.c runtime/runtime_h3.c runtime/runtime_quic.c"

snapshot() {
    mkdir -p "$SNAP"
    for f in $SRC_LIST; do mkdir -p "$SNAP/$(dirname "$f")"; cp "$f" "$SNAP/$f"; done
}
restore_all() {
    for f in $SRC_LIST; do [ -f "$SNAP/$f" ] && cp "$SNAP/$f" "$f"; done
}
trap 'restore_all; rm -rf "$W"' EXIT
snapshot

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad()  { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
step() { echo; echo "=== $1 ==="; }

# 就地替换（唯一匹配；否则报错）—— 只用于负控
patch_one() {   # $1=file $2=old $3=new $4=tag
    python3 - "$1" "$2" "$3" "$4" <<'PY' || exit 1
import sys
p, old, new, tag = sys.argv[1:5]
s = open(p, encoding='utf-8').read()
n = s.count(old)
if n != 1:
    print("  !! [%s] 锚点匹配 %d 次（要求 1）" % (tag, n)); sys.exit(1)
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PY
}

# 建探针（$1=源 $2=tag）→ $W/b<tag>/build/<name>
build_probe() {
    local src="$1"
    local tag="$2"
    local d="$W/b$tag"
    mkdir -p "$d"; cp "$src" "$d/"
    ( cd "$d" && timeout 900 "$ROOT/tools/px" build "$(basename "$src")" > "$W/$tag.log" 2>&1 ) || {
        echo "  (构建失败，日志尾)"; tail -4 "$W/$tag.log" | sed 's/^/      /'; return 1; }
    echo "$d/build/$(basename "${src%.px}")"
}

MIN_FUNCS=1500
MIN_KEEPS=200
BASE="$HERE/BASELINE.tsv"

step "① 静态：扫描器自证（19 锚点）"
if python3 selfhost/gcroot_audit.py --self-test > "$W/selftest.log" 2>&1; then
    # ⚠️ M213（第 92 轮）：锚点数 **16 → 19**（新增 hit9/hit10/miss9 —— 出口参数式
    #   构造器（缺陷 294-a）与「成员取址别名不是消费」（缺陷 294-b）的正/反向判据）。
    grep -q 'self-test: 19 通过 / 0 失败' "$W/selftest.log" && ok "自证 19/19（10 必中 + 9 必不中；M213 增 hit9/hit10/miss9）" \
        || { bad "自证结论行不符"; tail -8 "$W/selftest.log" | sed 's/^/      /'; }
else
    bad "自证脚本失败"; tail -6 "$W/selftest.log" | sed 's/^/      /'
fi

step "② 静态：候选 ⇄ 已判定基线"
python3 selfhost/gcroot_audit.py --json > "$W/scan.json" 2> "$W/scan.err" || bad "扫描器执行失败"
python3 - "$BASE" "$W/scan.json" "$MIN_FUNCS" "$MIN_KEEPS" <<'PY' > "$W/scan.chk" 2>&1
import json, sys
base_p, scan_p, min_funcs, min_keeps = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
d = json.load(open(scan_p, encoding='utf-8'))
rows = []
for ln in open(base_p, encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln or ln.startswith('#'):
        continue
    c = ln.split('\t')
    rows.append({'key': tuple(c[0:4]), 'verdict': c[4], 'reason': c[5] if len(c) > 5 else ''})
base_keys = [r['key'] for r in rows]
real = [r for r in rows if r['verdict'] != '假阳']
errs = []
if real:
    errs.append("基线里有 %d 条判定为「真」= 未修的缺陷: %s" % (len(real), [r['key'] for r in real]))
got = []
for f in d['findings']:
    v = ','.join(x['name'] for x in f['victims'])
    got.append((f['file'], f['func'], f['trigger'], v))
# 候选必须在基线在册（按多重集比较）
from collections import Counter
cg, cb = Counter(got), Counter(base_keys)
extra = cg - cb
missing = cb - cg
for k, n in extra.items():
    errs.append("**新增候选**（不在基线，疑似新缺陷）×%d: %s" % (n, k))
for k, n in missing.items():
    errs.append("基线在册但扫描不到（判据失牙/已修）×%d: %s" % (n, k))
if d['stats']['funcs'] < min_funcs:
    errs.append("规模下限失败：函数 %d < %d（扫描器可能静默失效）" % (d['stats']['funcs'], min_funcs))
if d['stats']['keeps'] < min_keeps:
    errs.append("规模下限失败：登记站点 %d < %d（= 本轮的修复被整体回退？）" % (d['stats']['keeps'], min_keeps))
print("候选 %d · 基线 %d · 函数 %d · 登记 %d · 分配 %d"
      % (len(got), len(base_keys), d['stats']['funcs'], d['stats']['keeps'], d['stats']['allocs']))
if errs:
    print("FAIL")
    for e in errs:
        print("   " + e)
    sys.exit(1)
print("OK")
PY
if grep -q '^OK$' "$W/scan.chk"; then
    ok "$(head -1 "$W/scan.chk")"
else
    bad "候选/基线不符"; sed -n '2,12p' "$W/scan.chk" | sed 's/^/      /'
fi

step "③ 动态：HTTP 面（urlencoded / multipart / session）"
BIN_RT="$(build_probe "$HERE/probe_rt.px" rt)" || bad "probe_rt 构建失败"
run_rt() {   # $1=env串 $2=tag
    local envs="$1"
    local tag="$2"
    if [ -n "$envs" ]; then eval "env $envs timeout 300 \"$BIN_RT\"" > "$W/rt.$tag.out" 2>&1
    else timeout 300 "$BIN_RT" > "$W/rt.$tag.out" 2>&1; fi
    local rc=$?
    if [ $rc -eq 0 ] && grep -q 'PROBE_RT OK' "$W/rt.$tag.out" && ! grep -q 'PX_GC_LIVECHK' "$W/rt.$tag.out"; then
        return 0
    fi
    echo "      rc=$rc"; tail -6 "$W/rt.$tag.out" | sed 's/^/      /'; return 1
}
run_rt "" base && ok "基线档 40/40 OK（无 LIVECHK）" || bad "基线档失败"
run_rt "PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1" stress \
    && ok "压力档 40/40 OK（无 LIVECHK / 无 core）" || bad "压力档失败（漏登记复现）"

step "④ 动态：UDP 面（udp_serve 的 hargs 族）"
BIN_UDP="$(build_probe "$HERE/probe_udp.px" udp)" || bad "probe_udp 构建失败"
run_udp() {
    local envs="$1"
    local tag="$2"
    if [ -n "$envs" ]; then eval "env $envs timeout 300 \"$BIN_UDP\"" > "$W/udp.$tag.out" 2>&1
    else timeout 300 "$BIN_UDP" > "$W/udp.$tag.out" 2>&1; fi
    local rc=$?
    if [ $rc -eq 0 ] && grep -q 'PROBE_UDP OK' "$W/udp.$tag.out" && ! grep -q 'PX_GC_LIVECHK' "$W/udp.$tag.out"; then
        return 0
    fi
    echo "      rc=$rc"; tail -6 "$W/udp.$tag.out" | sed 's/^/      /'; return 1
}
run_udp "" base && ok "基线档 20/20 OK" || bad "基线档失败"
run_udp "PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1" stress \
    && ok "压力档 20/20 OK（无 LIVECHK）" || bad "压力档失败"

if [ "$NEG_SKIP" = 1 ]; then
    step "⑤ 负控：--neg-skip（CI 环境跳过）"
    echo "  ⏭  跳过 3 道负控"
else
    step "⑤ 负控（每道独立：先还原 → 再打补丁 → 判红 → 再还原）"
    # A：撤 bi_udp_recv 的 KEEP(r) —— UDP 面（实测：撤掉后压力档必红）
    restore_all
    patch_one runtime/runtime.c '    px_root_push_keep(r);
    px_dict_set(r, "data", px_bytes_len(buf, n));' '    px_dict_set(r, "data", px_bytes_len(buf, n));' A || bad "负控 A 打补丁失败"
    if BIN_N="$(build_probe "$HERE/probe_udp.px" na)"; then
        if env PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1 timeout 300 "$BIN_N" > "$W/na.out" 2>&1 \
           && grep -q 'PROBE_UDP OK' "$W/na.out" && ! grep -q 'PX_GC_LIVECHK' "$W/na.out"; then
            bad "负控 A 未判红（撤 bi_udp_recv 的 KEEP 后压力档仍全绿 ⇒ 判据没牙）"
        else
            ok "负控 A 判红（撤 bi_udp_recv 的 KEEP ⇒ UDP 压力档失败）"
        fi
    else
        bad "负控 A 构建失败"
    fi
    restore_all
    cmp -s runtime/runtime.c "$SNAP/runtime/runtime.c" || bad "负控 A 后源未还原"

    # B：撤 px_session_read 的 KEEP(v)
    patch_one runtime/runtime.c "    LXValue v = px_str_len(data, len); PX_KEEP(v);" \
                                "    LXValue v = px_str_len(data, len);" B || bad "负控 B 打补丁失败"
    if BIN_N="$(build_probe "$HERE/probe_rt.px" nb)"; then
        if env PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1 timeout 300 "$BIN_N" > "$W/nb.out" 2>&1 \
           && grep -q 'PROBE_RT OK' "$W/nb.out" && ! grep -q 'PX_GC_LIVECHK' "$W/nb.out"; then
            bad "负控 B 未判红"
        else
            ok "负控 B 判红（撤 px_session_read 的 KEEP ⇒ 压力档失败）"
        fi
    else
        bad "负控 B 构建失败"
    fi
    restore_all
    cmp -s runtime/runtime.c "$SNAP/runtime/runtime.c" || bad "负控 B 后源未还原"

    # C：撤 h3_extra_to_headers 的 KEEP(pair) —— 静态判据
    patch_one runtime/runtime_h3.c "                    px_root_push_keep(pair);
" "" C || bad "负控 C 打补丁失败"
    if python3 selfhost/gcroot_audit.py --json > "$W/negc.json" 2>/dev/null; then
        if python3 -c "
import json,sys
d=json.load(open('$W/negc.json'))
n=sum(1 for f in d['findings'] if f['func']=='h3_extra_to_headers')
sys.exit(0 if n>=1 else 1)"; then
            ok "负控 C 判红（撤 KEEP(pair) ⇒ 静态扫描重新报出 h3_extra_to_headers）"
        else
            bad "负控 C 未判红（静态判据对该站点没牙）"
        fi
    else
        bad "负控 C 扫描失败"
    fi
    restore_all
    cmp -s runtime/runtime_h3.c "$SNAP/runtime/runtime_h3.c" || bad "负控 C 后源未还原"
fi

echo
echo "M206 门：通过 $PASS · 失败 $FAIL"
if [ "$FAIL" -eq 0 ]; then echo "M206-VERIFY-OK"; exit 0; fi
echo "M206-VERIFY-FAIL"; exit 1
