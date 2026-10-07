#!/usr/bin/env bash
if tail -60 /tmp/g.log | grep -qE '双路判据一致：0 红'; then echo green; fi
