#!/usr/bin/env bash
# ============================================================
# pxhwmond 启动 wrapper —— daemon 崩溃自动重启（MONITORING/P0 自愈）
# ------------------------------------------------------------
# 用法：run.sh [main 参数...]      （透传给 build/main）
# env：
#   PXHWMON_BIN         二进制路径（默认本目录 build/main）
#   PXHWMON_RESTART_MAX 崩溃重启上限（0=无限，默认 0）
# 行为：
#   - daemon 以非 0/130/143 退出码退出（崩溃/SIGKILL/kill -9）→ 1s 后自动拉起
#   - 退出码 0（--n 正常完成）/ 130 (SIGINT) / 143 (SIGTERM) → 正常退出不再重启
#   - 每次启动打日志（stdout/stderr 由调用方重定向，如 systemd/journald）
#
# systemd unit 示例（边缘设备部署）：
#   [Unit] Description=pxhwmond hardware health monitor
#   [Service] ExecStart=/opt/pxhwmond/run.sh --port 19858
#     Restart=always  Environment=PXHWMON_INTERVAL=5
#   [Install] WantedBy=multi-user.target
# ============================================================
set -u
cd "$(dirname "$0")"                  # run.sh 所在目录
BIN="${PXHWMON_BIN:-$PWD/build/main}"
RESTART_MAX="${PXHWMON_RESTART_MAX:-0}"

if [ ! -x "$BIN" ]; then
    echo "[run.sh] 未找到 $BIN —— 先构建：tools/pxc build examples/m58_hwmond/main.px" >&2
    exit 1
fi

N=0
while true; do
    N=$((N + 1))
    echo "[run.sh] 启动 pxhwmond (attempt $N, $(date +%H:%M:%S))"
    "$BIN" "$@"
    RC=$?
    echo "[run.sh] pxhwmond 退出 code=$RC @ $(date +%H:%M:%S)"
    # 优雅退出码（0 正常完成 / 130 SIGINT / 143 SIGTERM）→ 不再重启
    if [ "$RC" -eq 0 ] || [ "$RC" -eq 130 ] || [ "$RC" -eq 143 ]; then
        echo "[run.sh] 正常退出（$RC），不再重启"
        exit "$RC"
    fi
    # 崩溃（含 kill -9 → 137）：自动重启
    if [ "$RESTART_MAX" -gt 0 ] && [ "$N" -ge "$RESTART_MAX" ]; then
        echo "[run.sh] 达重启上限 $RESTART_MAX，放弃" >&2
        exit 1
    fi
    echo "[run.sh] 崩溃（code=$RC），1s 后自动重启..."
    sleep 1
done
