#!/usr/bin/env bash
# M130 验证：文件锁 / 权限 / open 原始 flags（qg-issue 87 缺陷 56–58）
# 依赖：tools/px（编译模式；python3 用于跨进程持锁对照）
# 两轨都跑（默认 VM 轨 + C 轨），断言集合相同。
set -u
cd "$(dirname "$0")"
DIR=$(pwd)
PX=../../tools/px
LOG=/tmp/m130_build.log
FAIL=0

run_track() {   # $1=标签 $2=额外环境/参数前缀（此处用 env 变量）
    local label="$1"; shift
    echo "== [$label] 编译 =="
    if ! "$@" "$PX" build flock_chmod.px > "$LOG" 2>&1; then
        echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    echo "== [$label] 运行 =="
    local out
    out=$(./build/flock_chmod 2>&1)
    local rc=$?
    echo "$out"
    echo "$out" | grep -q "M130_ASSERT: 12P/0F" || { echo "FAIL [$label] 断言计数不符（期望 12P/0F，rc=$rc）"; FAIL=$((FAIL+1)); return; }
    [ "$rc" = "0" ] || { echo "FAIL [$label] 退出码 $rc"; FAIL=$((FAIL+1)); return; }
    echo "PASS [$label]"
}

mkdir -p build
run_track "VM轨" env
run_track "C轨" env PX_BUILD_ENGINE=c

# 负控：把期望权限改成 0700，stat 断言必须变红（门自欺检测）
echo "== 负控 =="
sed 's/chmod(lockpath, 0o600)/chmod(lockpath, 0o700)/' flock_chmod.px > /tmp/m130_neg.px
if "$PX" build /tmp/m130_neg.px > "$LOG" 2>&1 && /tmp/build/m130_neg > /tmp/m130_neg.out 2>&1; then
    echo "FAIL 负控未生效（改权限后仍全绿，门在自欺）"; FAIL=$((FAIL+1))
else
    echo "PASS 负控生效（0700 时 600 断言变红）"
fi

echo ""
if [ "$FAIL" = "0" ]; then
    echo "M130-VERIFY-OK"
    exit 0
fi
echo "M130-VERIFY-FAIL: $FAIL 项"
exit 1
