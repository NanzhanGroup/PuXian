#!/usr/bin/env bash
# M134 验证：GC 根面必须覆盖全部堆对象类型（qg-issue 87 缺陷 86 根治 + 复发防线）
#
#   缺陷 86（runtime）：`px_value_is_obj` 白名单漏 PX_MUTEX / PX_RWLOCK / PX_GEN
#     ⇒ 这三种值放进 list/dict/struct/gen/chan 或作为全局/帧槽的直接值时**不被 GC
#     标记** ⇒ 对象被 sweep 回收（悬垂）。实测症状：token-cache 生产链路上第 6 个
#     「走审批判定」的请求起**永久挂死**（pthread_mutex_lock 阻塞在已回收内存的
#     __lock 上，无持有者、无唤醒源）。
#   修法：改为 `g_type_is_obj[PX_TYPE_MAX]` 位置表 + `_Static_assert` 尺寸断言。
#
# 本门三件事：
#   ① 行为门：把每种堆对象放进容器/全局/帧 → gc() 强制整轮回收 → 逐个使用（两轨都跑）
#   ② 静态门：check_whitelist.py 读源码文本核对「枚举成员全集 ↔ 表条目」
#   ③ 负控：把表中 PX_MUTEX 改成 false 的**副本**必须被判红（门自欺检测）
set -u
cd "$(dirname "$0")"
PX=../../tools/px
RT=../../runtime
LOG=/tmp/m134_build.log
FAIL=0

run_track() {   # $1=标签；$2..=编译命令前缀（env [PX_BUILD_ENGINE=c]）
    local label="$1"; shift
    echo "== [$label] 编译 =="
    if ! "$@" "$PX" build gc_obj_roots.px > "$LOG" 2>&1; then
        echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    echo "== [$label] 运行 =="
    local out rc
    out=$(./build/gc_obj_roots 2>&1); rc=$?
    echo "$out" | sed 's/^/     /'
    echo "$out" | grep -q "M134_ASSERT: 17P/0F" || {
        echo "FAIL [$label] 断言计数不符（期望 17P/0F）"; FAIL=$((FAIL+1)); return; }
    [ "$rc" = "0" ] || { echo "FAIL [$label] 退出码 $rc"; FAIL=$((FAIL+1)); return; }
    echo "PASS [$label]"
}

mkdir -p build
run_track "VM轨" env
run_track "C轨" env PX_BUILD_ENGINE=c

echo "== [静态门] 类型表 ↔ LXType 枚举 =="
if python3 check_whitelist.py "$RT/runtime.c" "$RT/runtime.h"; then
    echo "PASS [静态门]"
else
    echo "FAIL [静态门]"; FAIL=$((FAIL+1))
fi

echo "== [负控] 把表中 PX_MUTEX 改成 false 的副本必须判红 =="
sed 's|/\* PX_MUTEX  \*/ true|/* PX_MUTEX  */ false|' "$RT/runtime.c" > /tmp/m134_neg_runtime.c
if grep -q '/\* PX_MUTEX  \*/ false' /tmp/m134_neg_runtime.c; then
    if python3 check_whitelist.py /tmp/m134_neg_runtime.c "$RT/runtime.h" > /tmp/m134_neg.out 2>&1; then
        echo "FAIL 负控未生效（改 false 后静态门仍绿 = 门在自欺）"; FAIL=$((FAIL+1))
    else
        echo "PASS 负控生效：$(grep -m1 '·' /tmp/m134_neg.out)"
    fi
else
    echo "FAIL 负控准备失败（sed 未命中表项，模式已漂移）"; FAIL=$((FAIL+1))
fi

echo "== [负控2] 表漏一项的副本必须判红 =="
sed 's|    /\* PX_RWLOCK \*/ true,||' "$RT/runtime.c" > /tmp/m134_neg2_runtime.c
if python3 check_whitelist.py /tmp/m134_neg2_runtime.c "$RT/runtime.h" > /tmp/m134_neg2.out 2>&1; then
    echo "FAIL 负控2 未生效（删表项后仍绿）"; FAIL=$((FAIL+1))
else
    echo "PASS 负控2 生效：$(grep -m1 '·' /tmp/m134_neg2.out)"
fi

echo ""
if [ "$FAIL" = "0" ]; then
    echo "M134-VERIFY-OK"
    exit 0
fi
echo "M134-VERIFY-FAIL: $FAIL 项"
exit 1
