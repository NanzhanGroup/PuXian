#!/usr/bin/env bash
bash selfhost/run_gates.sh > /tmp/g.log 2>&1
/usr/local/sbin/dy-notify.sh resolve-topic m284 >/dev/null 2>&1 || true
