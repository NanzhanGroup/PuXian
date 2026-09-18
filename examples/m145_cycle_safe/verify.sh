#!/usr/bin/env bash
# M145 验证：循环引用值的**比较/渲染/JSON** 不再段错误（qg-issue 87 第 26 轮 · 缺陷 125）
#   缺陷 125：`var g = {}; g.set("me", g)` 之后
#     · `==`            → compare_values 无限递归 ⇒ C 栈溢出 ⇒ SIGSEGV（实测 rc=139，双轨同源）
#     · `str()`         → px_fmt_value 同族（编译轨）/ ival.px i_to_str（解释轨）
#     · `json_stringify`→ json_stringify_value 同族
#   契约：比较对齐 Go reflect.DeepEqual（环上「已访问对象对」再遇 ⇒ 相等）；
#         渲染按**路径**判定（共享而非环的对象必须完整渲染）⇒ 环上渲染 `...`；
#         JSON 在环上**受控报错**（Go encoding/json 同族文案）。
#   三轨断言集相同：VM（默认 build）/ C（PX_BUILD_ENGINE=c）/ 解释轨（px run）。
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/px
FAIL=0
PROGS="cycle_eq render object_id"

run_track() {   # $1 = 轨名（vm / c / pxi）
    local track="$1"
    echo "== 轨：$track =="
    for prog in $PROGS; do
        local log="/tmp/m145_${track}_${prog}_build.log"
        local out="/tmp/m145_${track}_${prog}.txt"
        local ok=1
        if [ "$track" = "pxi" ]; then
            "$PX" run "$prog.px" > "$out" 2>&1 || ok=$?
        elif [ "$track" = "c" ]; then
            env PX_BUILD_ENGINE=c "$PX" build "$prog.px" > "$log" 2>&1 || ok=0
            if [ "$ok" = "1" ]; then "./build/$prog" > "$out" 2>&1 || ok=$?; else ok=0; fi
        else
            "$PX" build "$prog.px" > "$log" 2>&1 || ok=0
            if [ "$ok" = "1" ]; then "./build/$prog" > "$out" 2>&1 || ok=$?; else ok=0; fi
        fi
        sed "s/^/     [$prog] /" "$out" 2>/dev/null
        if [ "$ok" != "1" ]; then
            echo "FAIL 运行失败（$track / $prog，rc=$ok）"; [ -f "$log" ] && tail -20 "$log"; FAIL=$((FAIL+1)); continue
        fi
        if grep -q '^FAIL ' "$out"; then
            echo "FAIL 存在失败断言（$track / $prog）"; grep '^FAIL ' "$out"; FAIL=$((FAIL+1)); continue
        fi
        local np
        np=$(grep -c '^PASS ' "$out" || true)
        if [ "$np" -lt 5 ]; then
            echo "FAIL PASS 行数过少（$track / $prog，n=$np）—— 防空转"; FAIL=$((FAIL+1))
        fi
    done
}

run_track vm
run_track c
run_track pxi

echo "--- 三轨输出逐字节一致性 ---"
for prog in $PROGS; do
    if diff -q "/tmp/m145_vm_${prog}.txt" "/tmp/m145_c_${prog}.txt" > /dev/null 2>&1; then
        echo "PASS VM/C 一致（$prog）"
    else
        echo "FAIL VM/C 不一致（$prog）："; diff "/tmp/m145_vm_${prog}.txt" "/tmp/m145_c_${prog}.txt" | head -20; FAIL=$((FAIL+1))
    fi
    if diff -q "/tmp/m145_vm_${prog}.txt" "/tmp/m145_pxi_${prog}.txt" > /dev/null 2>&1; then
        echo "PASS VM/解释轨 一致（$prog）"
    else
        echo "FAIL VM/解释轨 不一致（$prog）："; diff "/tmp/m145_vm_${prog}.txt" "/tmp/m145_pxi_${prog}.txt" | head -20; FAIL=$((FAIL+1))
    fi
done

echo "--- 环上的 JSON 序列化必须是**受控报错**（不是段错误）---"
for track in vm c pxi; do
    log="/tmp/m145_${track}_json_cycle.txt"
    if [ "$track" = "pxi" ]; then
        "$PX" run json_cycle.px > "$log" 2>&1; rc=$?
    else
        if [ "$track" = "c" ]; then env PX_BUILD_ENGINE=c "$PX" build json_cycle.px >/dev/null 2>&1; else "$PX" build json_cycle.px >/dev/null 2>&1; fi
        "./build/json_cycle" > "$log" 2>&1; rc=$?
    fi
    if [ "$rc" = "139" ]; then
        echo "FAIL 环上 json 段错误（$track，rc=139）"; FAIL=$((FAIL+1))
    elif [ "$rc" != "1" ]; then
        echo "FAIL 环上 json 期望 rc=1，实际 rc=$rc（$track）"; tail -5 "$log"; FAIL=$((FAIL+1))
    elif ! grep -q 'encountered a cycle' "$log"; then
        echo "FAIL 环上 json 缺环报告文案（$track）"; tail -5 "$log"; FAIL=$((FAIL+1))
    elif grep -q '^AFTER-SERIALIZE' "$log"; then
        echo "FAIL 环上 json 竟然序列化成功（$track）"; FAIL=$((FAIL+1))
    else
        echo "PASS 环上 json 受控报错（$track）：$(tail -1 "$log")"
    fi
done

echo "--- 负控：篡改期望值 ⇒ 门必须判红 ---"
python3 - "$PX" <<'PYEOF'
import subprocess, sys, os, shutil
px = sys.argv[1]
src = "cycle_eq.px"
tmp = "/tmp/m145_neg_cycle_eq.px"
s = open(src, encoding="utf-8").read()
assert 'str' not in s or True
# 把「自环 dict 与自身相等」的期望翻成 false（真实值是 true）⇒ 必须 FAIL
bad = s.replace('ck("A1 自环 dict 与自身相等", g == g)', 'ck("A1 自环 dict 与自身相等", false)')
assert bad != s
open(tmp, "w", encoding="utf-8").write(bad)
r = subprocess.run([px, "run", tmp], capture_output=True, text=True)
if r.returncode != 1 or "FAIL A1" not in r.stdout:
    print("FAIL 负控未判红（rc=%d）" % r.returncode); sys.exit(1)
print("PASS 负控：篡改期望值 ⇒ FAIL A1 + rc=1")
# 负控 2：把「路径语义」换成「累积集合语义」的等价断言 —— 共享对象**不得**渲染成 ...
r2 = open("render.px", encoding="utf-8").read()
if 'B1 共享子对象完整渲染' not in r2 or 'str(shared) == "{p: {x: 1}, q: {x: 1}}"' not in r2:
    print("FAIL 负控 2 找不到「共享对象完整渲染」断言"); sys.exit(1)
print("PASS 负控 2：路径语义断言在位（共享对象须完整渲染 ⇒ 累积集合语义会被本门拦下）")
PYEOF
[ $? -ne 0 ] && FAIL=$((FAIL+1))

if [ "$FAIL" = "0" ]; then echo "✅ M145 门：全部通过"; else echo "❌ M145 门：$FAIL 项失败"; fi
exit $FAIL
