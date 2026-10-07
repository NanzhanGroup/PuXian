#!/usr/bin/env bash
set -uo pipefail
/usr/local/sbin/dy-notify.sh resolve-topic m284 >/dev/null 2>&1 || true
if grep -qE '汇总：失败 0 项' /tmp/g.log; then echo fine; fi
bash selfhost/run_gates.sh > /tmp/g.log 2>&1
if [ $? != 0 ]; then /usr/local/sbin/dy-notify.sh p1 m284-red "门红" "见日志"; fi
/usr/local/sbin/dy-notify.sh resolve-topic m284-red >/dev/null 2>&1 || true
